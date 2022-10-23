#include "usb_custom.h"
#include "pcd_state.h"
#include "hf_module.h"
#include "MFRC522.h"

custom_usb_message g_usb_message = {0};
uint8_t g_usb_tx_buf[MAX_USB_BUF] = {0};
command_vector g_cmd_table[MAX_CMD] = {0};
static uint8_t sg_usb_rxpayload_buf[MAX_USB_BUF] = {0};
static uint8_t sg_usb_txpayload_buf[MAX_USB_BUF] = {0};
static uint8_t sg_trans_power = 0;

static void cmd_table_init(void);
static uint8_t action_connect(msg_data *, msg_data *);
static uint8_t action_read(msg_data *, msg_data *);
static uint8_t action_write(msg_data *, msg_data *);
static uint8_t action_buzzer_test(msg_data *, msg_data *);
static uint8_t action_led_test(msg_data *, msg_data *);
static uint8_t action_pasm_test(msg_data *, msg_data *);
static uint8_t action_set_trans_power(msg_data *, msg_data *);
static uint8_t action_read_trans_power(msg_data *, msg_data *);

void usd_custom_init(void)
{
    cmd_table_init();
    hf_sm_init();
}

void parse_packet(uint8_t *Buf, uint32_t Len, custom_usb_message *message)
{
    uint8_t ret = FALSE;
    uint8_t *pCur = Buf;
    uint16_t payload_len  = Len - PACKET_LEN_MIN;
    msg_data *tmp_payload = &(message->payload);
    
    // big-endian to little-endian
    message->head = (((uint16_t)*(pCur)) << 8) + *(pCur + 1);
    pCur += HEAD_OFFSET;
    message->len = *(pCur++);
    message->cmd = *(pCur++);
    message->subcmd = *(pCur++);
    // pCur -> packet's data field now
    tmp_payload->data_len = payload_len;
    message->check = *(pCur + payload_len);
    
    memset(sg_usb_rxpayload_buf, 0, MAX_BUF_LEN);
    if (payload_len != 0)
    {
        memcpy(sg_usb_rxpayload_buf, pCur, payload_len);
        tmp_payload->pdata = sg_usb_rxpayload_buf;
    }
    else
    {
        tmp_payload->pdata = NULL;
    }
    
    ret = validate(message);
    message->valid = (ret == TRUE) ? TRUE : FALSE;
}

uint16_t assemble_packet(custom_usb_message *message, uint8_t *tx_buf)
{
    uint8_t *ptx_buf = tx_buf;
    uint16_t data_length = message->payload.data_len;
    
    // little-endian to big-endian
    *(ptx_buf++) = (uint8_t)((message->head & 0xFF00) >> 8);
    *(ptx_buf++) = (uint8_t)(message->head & 0x00FF);
    
    *(ptx_buf++) = message->len;
    *(ptx_buf++) = message->cmd;
    *(ptx_buf++) = message->subcmd;
    
    if (data_length > 0)
    {
        memcpy(ptx_buf, message->payload.pdata, data_length);
    }
    ptx_buf += data_length;
    
    *(ptx_buf++) = message->check;
    
    return ptx_buf - g_usb_tx_buf;
}

uint8_t checksum(custom_usb_message *message)
{
    uint8_t sum = 0; 
    uint16_t msg_head = message->head;
    uint8_t *pCur_data = message->payload.pdata;
    uint16_t msg_data_len = message->payload.data_len;
    uint8_t head_low  = (uint8_t)(msg_head & 0x00ff);
    uint8_t head_high = (uint8_t)((msg_head & 0xff00) >> 8);
    
    sum = head_low + head_high + message->len + message->cmd
          + message->subcmd;
    
    for (uint8_t i = 0; i < msg_data_len; i++)
    {
        sum += *(pCur_data++);
    }
    
    sum = (~sum) + 1;  

    return sum;
}

uint8_t validate(custom_usb_message *message)
{
    uint8_t check_sum = 0;
    uint16_t len_field = message->len;
    
    check_sum = checksum(message);
    
    if ((message->head != PREAMBLE) || (len_field < LEN_MIN)
        || (message->cmd >= MAX_CMD) || (message->subcmd != DOWN_STREAM) 
        || (message->check != check_sum) || (len_field != (message->payload.data_len + LEN_MIN)))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }        
}

/******************************************************************
************************* USB COMMAND *****************************
******************************************************************/
static uint8_t action_connect(msg_data *in_data, msg_data *out_data)
{
    //HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    
    HAL_TIM_Base_Start_IT(&htim2);
    
    if (g_hf_sm.currentState == &g_hf_offline)
    {
        stateM_handleEvent(&g_hf_sm, &(struct event){EVT_CONNECT, NULL});
    }
    
    out_data->pdata = NULL;
    out_data->data_len = 0;
    
    return NACK;
}

static uint8_t action_read(msg_data *in_data, msg_data *out_data)
{   
    if (g_hf_sm.currentState != &g_hf_offline)
    { 
        uint8_t status = MI_ERR;
        
        status = MFRC522_Request(PICC_REQALL, NULL);      
        if (status == MI_OK)
        {
            status = tree_creater(&g_card_tbl, sizeof(card_hf_uid));
            if (status == MI_OK)
            {
                status = anti_collision_loop(g_card_tbl, 1);
                if (status & MI_TAGFOUND)
                {
                    HF_TRAVERSE_CALLBACK(leaf, g_card_tbl, cb_hf_module_multiRead);
                }
            }
        }
    }
    
    out_data->pdata = NULL;
    out_data->data_len = 0;
    
    return NACK;
}

static uint8_t action_write(msg_data *in_data, msg_data *out_data)
{
    if (g_hf_sm.currentState != &g_hf_offline)
    { 
        uint8_t status = MI_ERR;
        uint8_t *uid   = g_hf_uid.UID;
        uint8_t block  = BLOCK_ID;
        uint8_t block_data[BLOCK_LEN] = {0};
        uint8_t rx_data_len = in_data->data_len;
        
        if (rx_data_len > BLOCK_LEN)
        {
            status = MI_ERR;
        }
        else
        {
            memcpy(block_data, in_data->pdata, rx_data_len);
        
            if (g_hf_sm.currentState != &g_hf_auth)
            {
                uint8_t d_ATQA[2] = {0};
            
                status = MFRC522_Request(PICC_REQIDL, d_ATQA);
            
                if (status == MI_OK)
                {
                    stateM_handleEvent(&g_hf_sm, &(struct event){EVT_REQA, NULL});
                    status = MFRC522_Anticoll(uid);
                
                    if (status == MI_OK)
                    {
                        status = MFRC522_SelectTag(uid);
                        if (status > 0)
                        {
                            stateM_handleEvent(&g_hf_sm, &(struct event){EVT_SELECT, NULL});
                            status = MFRC522_Auth(PICC_AUTHENT1A, block, KEY_A_DEFAULT, uid);
                            if (status == MI_OK)
                            {
                                stateM_handleEvent(&g_hf_sm, &(struct event){EVT_AUTH, NULL});
                                status = MFRC522_Write(block, block_data);
                            }
                        }
                    }
                }
            }
            else
            {
                status = MFRC522_Write(block, block_data);
            }
        }
        
        if (status != MI_ERR)
        {
            sg_usb_txpayload_buf[0] = CMD_SUCCESS;
        }
        else
        {
            stateM_handleEvent(&g_hf_sm, &(struct event){EVT_ERR, NULL});
            sg_usb_txpayload_buf[0] = CMD_FAIL;
        }
        
        out_data->pdata = sg_usb_txpayload_buf;
        out_data->data_len = 1;
        
        return ACK;
    }
        
    return NACK;
}

static uint8_t action_buzzer_test(msg_data *in_data, msg_data *out_data)
{
    //uint8_t ret = NACK;
    
    if (g_hf_sm.currentState != &g_hf_offline)
    {
    /****
        ret = HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
        if (ret != HAL_OK)
        {
            HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
        }
    ****/
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
    
        sg_usb_txpayload_buf[0] = CMD_SUCCESS;
        
        out_data->pdata = sg_usb_txpayload_buf;
        out_data->data_len = 1;
    
        return ACK;
    }
        
    return NACK;
}

static uint8_t action_led_test(msg_data *in_data, msg_data *out_data)
{
    if (g_hf_sm.currentState != &g_hf_offline)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
    
        sg_usb_txpayload_buf[0] = CMD_SUCCESS;
    
        out_data->pdata = sg_usb_txpayload_buf;
        out_data->data_len = 1;
    
        return ACK;
    }
        
    return NACK;
}

static uint8_t action_pasm_test(msg_data *in_data, msg_data *out_data)
{
    /* fix me! */
    
    sg_usb_txpayload_buf[0] = CMD_SUCCESS;
    
    out_data->pdata = sg_usb_txpayload_buf;
    out_data->data_len = 1;
    
    return ACK;
}

static uint8_t action_set_trans_power(msg_data *in_data, msg_data *out_data)
{
    /* fix me! */
    if (in_data->data_len == 1)
    {
        sg_usb_txpayload_buf[0] = CMD_SUCCESS;
        sg_trans_power = *(in_data->pdata);
    }
    else
    {
        sg_usb_txpayload_buf[0] = CMD_FAIL;
    }
    
    out_data->pdata = sg_usb_txpayload_buf;
    out_data->data_len = 1;
    
    return ACK;
}

static uint8_t action_read_trans_power(msg_data *in_data, msg_data *out_data)
{   
    sg_usb_txpayload_buf[0] = sg_trans_power;
    
    out_data->pdata = sg_usb_txpayload_buf;
    out_data->data_len = 1;
    
    return ACK;
}

static void cmd_table_init(void)
{
    REGISTER_COMMAND(g_cmd_table, CONNECT_CMD, NULL, action_connect);
    REGISTER_COMMAND(g_cmd_table, READ_CMD, NULL, action_read);
    REGISTER_COMMAND(g_cmd_table, WRITE_CMD, NULL, action_write);
    REGISTER_COMMAND(g_cmd_table, BUZZER_TEST, NULL, action_buzzer_test);
    REGISTER_COMMAND(g_cmd_table, LED_TEST, NULL, action_led_test);
    REGISTER_COMMAND(g_cmd_table, PASM_TEST, NULL, action_pasm_test);
    REGISTER_COMMAND(g_cmd_table, SET_TRANS_POWER, NULL, action_set_trans_power);
    REGISTER_COMMAND(g_cmd_table, READ_TRANS_POWER, NULL, action_read_trans_power);  
}
