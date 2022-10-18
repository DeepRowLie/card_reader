#include "thm3070_drv.h"

uint8_t Key_A[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t Key_B[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static uint8_t thm3070_reg_init(void);
static void thm3070_write_databuff(uint8_t *, uint16_t);
static void thm3070_read_databuff(uint8_t *, uint16_t);
// the following interface can be public
static uint8_t thm3070_sendFrame(uint8_t *, uint16_t);
static uint8_t thm3070_recvFrame(uint8_t *, uint16_t *);

void thm3070_write_reg(thm3070_reg_addr reg, uint8_t data)
{
    uint8_t addr_t  = reg | 0x80;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // enable nss
    
    // before spi data line is valid, delay 1us
    HAL_SPI_Transmit(&hspi1, &addr_t, 1, 500);
    HAL_SPI_Transmit(&hspi1, &data, 1, 500);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);  // disable nss
    
    return;
}

uint8_t thm3070_read_reg(thm3070_reg_addr reg)
{
    uint8_t rx_data = ADDR_MAX & 0x7F;
    uint8_t addr_t  = reg & 0x7F;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // enable nss
    
    // before spi data line is valid, delay 1us
    HAL_SPI_Transmit(&hspi1, &addr_t, 1, 500);
    HAL_SPI_Receive(&hspi1, &rx_data, 1, 500);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);  // disable nss
    
    return rx_data;
}

// thm3070 DATA BUFF must be read or written continuously
static void thm3070_write_databuff(uint8_t *data, uint16_t data_len)
{
    uint8_t addr_t  = DATA | 0x80;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // enable nss
    
    // before spi data line is valid, delay 1us
    HAL_SPI_Transmit(&hspi1, &addr_t, 1, 500);
    HAL_SPI_Transmit(&hspi1, data, data_len, 500);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);  // disable nss
    
    return;
}

static void thm3070_read_databuff(uint8_t *data, uint16_t data_len)
{
    uint8_t addr_t  = DATA & 0x7F;
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // enable nss
    
    // before spi data line is valid, delay 1us
    HAL_SPI_Transmit(&hspi1, &addr_t, 1, 500);
    HAL_SPI_Receive(&hspi1, data, data_len, 500);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);  // disable nss
    
    return;
}

void SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp;
    tmp = thm3070_read_reg(reg);
    thm3070_write_reg(reg, tmp | mask);  // set bit mask
}

void ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp;
    tmp = thm3070_read_reg(reg);
    thm3070_write_reg(reg, tmp & (~mask));  // clear bit mask
}

static uint8_t thm3070_sendFrame(uint8_t *p_tx_data, uint16_t data_len)
{
    uint8_t *p_tmp =  p_tx_data;
    uint32_t time_wait_TXFIN = 0;
    
    SetBitMask(SCON, BIT2);    // clear thm3070 data buff
    ClearBitMask(SCON, BIT2);  // data buff enable
    SetBitMask(EMVERR, BIT0);  // clear A_FDT_PICC_INVALID flag
    
    thm3070_write_databuff(p_tx_data, data_len);
    
    SetBitMask(SCON, BIT1);    // transmission begin
    
    // wait until TX finish
    while(!(thm3070_read_reg(TXFIN) & 0x03))
    {
        ++time_wait_TXFIN;
        if (time_wait_TXFIN > MAX_WAIT_TIME)
        {
            break;
        }
    }
    
    return CODE_OK;
}

static uint8_t thm3070_recvFrame(uint8_t *p_data, uint16_t *data_len)
{
    uint8_t EMV_Error = 0;
    uint8_t RxStatus  = 0;
    uint32_t time_wait_RXcpl = 0;
    
    // wait until RX complete or RX error  
    while(1)
    {
        EMV_Error = thm3070_read_reg(EMVERR);
        RxStatus  = thm3070_read_reg(RSTAT);
        
        // if RX error 
        if (((RxStatus & 0x7E) != 0) || ((EMV_Error & 0x03) != 0))
        {
            printf("ERROR!:thm3070 RxStatus or EMVERR wrong!\r\n");
            //printf("DEBUG:RxStatus = 0x%x, EMVERR = 0x%x", RxStatus, EMV_Error);
            return CODE_ERR;
        }
        else if ((RxStatus & 0x01) != 0)
        {
            break; // RX complete
        }
        
        ++time_wait_RXcpl;
        if (time_wait_RXcpl > MAX_WAIT_TIME)
        {
            printf("WARNING!:thm3070 RX takes too much time!\r\n");
            return CODE_ERR; // RX timeout
        }
    }
    
    *data_len = thm3070_read_reg(RSCH);
    *data_len = (*data_len << 8) | thm3070_read_reg(RSCL);
    if (*data_len > 256)
    {
        printf("ERROR!:thm3070 DATA buff overflow!\r\n");
        return CODE_ERR; // DATA BUFF overflow
    }
    else if (*data_len > 0)
    {
        thm3070_read_databuff(p_data, *data_len);
    }
    
    thm3070_write_reg(RSTAT, 0);
    
    return CODE_OK;
}

void thm3070_AntennaOn(void)
{
    ClearBitMask(TXCON, 0x01); // bit0 SHD clear
    return;
}

void thm3070_AntennaOff(void)
{
    SetBitMask(TXCON, 0x01); // bit0 SHD set
    return;
}

void thm3070_reset(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET); // clear PB15(RSTN) for 5ms
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);   // set PB15(RSTN)
    
    return;
}

static uint8_t thm3070_reg_init(void)
{
    uint8_t ret = CODE_OK;
    
    // common register init
    thm3070_write_reg(CRCSEL, 0x01);  // disable tx&rx crc
    //thm3070_write_reg(INTCON, 0x01);// default value
    thm3070_write_reg(EMVEN, 0xfd);   // enable noise detection
    thm3070_write_reg(RXCON, 0x42);   // change RX circuit's magnification to 40dB
    thm3070_setFWT(0x64);             // set FWT 100 * 330us = 33ms
    
    ret = (thm3070_read_reg(RXCON) == 0x42) ? CODE_OK : CODE_ERR;
    
    return ret;
}

uint8_t thm3070_Init(void)
{
    uint8_t ret = CODE_OK;
    
    thm3070_reset();
    thm3070_reg_init();         // common init
    thm3070_setMode_14443A();
    //thm3070_setMode_mifare();   // mode specified init
    thm3070_AntennaOn();
    
    return CODE_OK;
}

uint8_t thm3070_setMode_mifare(void)
{
    uint8_t ret = CODE_OK;
    
    thm3070_write_reg(TXCON, 0x72);    // enable TX1&TX2, ASK =100%, enable tx circuit
    thm3070_write_reg(PSEL, 0x50);     // select 14443 TYPE_A,tx&rx baud rate 106kbps
    
    return ret;
}

uint8_t thm3070_setMode_14443A(void)
{
    uint8_t ret = CODE_OK;
    
    thm3070_write_reg(TXCON, 0x72);    // enable TX1&TX2, ASK =100%, enable tx circuit
    thm3070_write_reg(PSEL, 0x10);     // select 14443 TYPE_A,tx&rx baud rate 106kbps
    
    return ret;
}

uint8_t thm3070_setMode_14443B(void)
{
    uint8_t ret = CODE_OK;
    
    /*************
    ****fix me!***
    *************/
    
    return ret;
}

uint8_t thm3070_setMode_15693(void)
{
    uint8_t ret = CODE_OK;
    
    /*************
    ****fix me!***
    *************/
    
    return ret;
}

/*
 * Function Name: thm3070_setFWT
 * Description: FWT = 330us * {FWIHIGH, FWIMID, FWILOW}
 * Input: fwt
 * Return value: status code
 */
uint8_t thm3070_setFWT(uint32_t fwt)
{
    uint8_t ret = CODE_OK;
    
    thm3070_write_reg(FWIHIGH, (fwt >> 16) & 0xFF);
    thm3070_write_reg(FWIMID, (fwt >> 8) & 0xFF);
    thm3070_write_reg(FWILOW, fwt & 0xFF);
    
    return ret;
}

/*
 * Function Name: thm3070_setFrameFormat
 * Description: among data frames of ISO/IEC14443 TYPE A/B,
 *              not all frames need crc.DISABLE CRC then.
 * Input: format
 * Return value: void
 */
void thm3070_setFrameFormat(uint8_t format)
{
    if (format == SHORT_FRM)
    {
        // short frame
        thm3070_write_reg(FM_CTRL, 0xC0);     //Reset Crypto1
        thm3070_write_reg(STAT_CTRL, 0x00);   // don't care FDT, don't Encrypt
        thm3070_write_reg(FM_CTRL, 0x40);     //Short Frame
        thm3070_write_reg(CRCSEL, 0x01); 
    }
    else if (format == BIT_ORIENTED)
    {
        // bit-oriented frames
        thm3070_write_reg(FM_CTRL, 0x46);     //ANTIcollision command,Bit Oriented Frame
        thm3070_write_reg(CRCSEL, 0x01);
    }
    else
    {
        // standard frame
        thm3070_write_reg(FM_CTRL, 0x42);
        thm3070_write_reg(CRCSEL, 0xC1);
    }
}

/***************************************************
****************** ISO/IEC14443 ********************
***************************************************/
uint8_t thm3070_request(uint8_t reqMode)
{
    uint8_t ret       = CODE_OK;
    uint8_t req[5]    = {0};
    uint16_t req_len  = 0;
    uint8_t atq[15]   = {0};
    uint16_t atq_len  = 0; // The received data bits
    
    if (reqMode == REQA)
    {
        req[0] = reqMode; // valid bits of atq is 7,set the bit7
        req_len = 1;

        thm3070_setFrameFormat(SHORT_FRM);
        thm3070_sendFrame(req, req_len);
        ret = thm3070_recvFrame(atq, &atq_len);
    
        if ((ret != CODE_OK) || (atq_len != ATQA_LEN)) 
        {
            ret = CODE_ERR;
        }
    }

    return ret;
}

uint8_t thm3070_Anticoll(uint8_t *serNum, uint8_t *serNum_len)
{
    uint8_t ret         = CODE_OK;  
    uint8_t serNumCheck = 0;
    uint8_t select[2]   = {0};
    uint16_t NVB        = 0x20;
    
    select[0] = SEL_C1;
    select[1] = NVB;
    
    thm3070_setFrameFormat(BIT_ORIENTED);
    thm3070_sendFrame(select, NVB >> 4);
    ret = thm3070_recvFrame(serNum, (uint16_t *)serNum_len);

    if (ret == CODE_OK)
    {
        //Check card serial number
        for (uint8_t i = 0; i < *serNum_len - 1; i++)
        {
            serNumCheck ^= serNum[i];
        }

        if (serNumCheck != serNum[*serNum_len - 1])
        {
            ret = CODE_ERR;
        }
    }

    return ret;
}

uint8_t thm3070_selectTag(uint8_t *serNum, uint8_t serNum_len)
{
    uint8_t ret       = CODE_OK;
    uint8_t SAK       = 0;
    uint8_t SAK_len   = 0;
    uint8_t select[7] = {0};
    uint16_t NVB      = 0x70;
    
    select[0] = SEL_C1;
    select[1] = NVB;
    for (uint8_t i = 0; i < serNum_len; i++)
    {
        select[i + 2] = serNum[i];
    }
    
    thm3070_setFrameFormat(STD_FRM);
    // thm3070_write_reg(CRCSEL, 0xC1);
    // thm3070_setFrameFormat(CRC_EN);  // enable tx&rx crc
    thm3070_sendFrame(select, NVB >> 4);
    ret = thm3070_recvFrame(&SAK, (uint16_t *)&SAK_len);
    
    // thm3070_write_reg(CRCSEL, 0x01);
    // thm3070_setFrameFormat(CRC_DIS);  // disable tx&rx crc
    
    return ret;
}

uint8_t thm3070_mifareAuth(uint8_t auth_keyType, uint8_t blockNum, uint8_t *UID)
{
    uint8_t ret          = CODE_OK;
    uint8_t auth[2]      = {auth_keyType, blockNum};
    uint8_t token[8]     = {0};
    uint8_t rx_data_len  = 0;
    uint16_t retry_times = 0;
    uint8_t auth_result  = 0;
    uint8_t *pKey        = ((auth_keyType == auth_withKeyA) ? Key_A : Key_B);
       
    thm3070_write_reg(0x1d, 0x00);       // register(0x1d):single-pass  
    thm3070_write_reg(CRCSEL, 0x80); 
    
    thm3070_sendFrame(auth, 2);
    ret = thm3070_recvFrame(token, (uint16_t *)&rx_data_len);
    if (rx_data_len != 4)
    {
        return CODE_ERR;
    }
    
    thm3070_write_reg(CRYPTO1_CTRL,0x08);    // Enable Cyrpto1 
    // record Token RB
	thm3070_write_reg(DATA1, token[0]);
	thm3070_write_reg(DATA2, token[1]);
	thm3070_write_reg(DATA3, token[2]);
	thm3070_write_reg(DATA4, token[3]);
    
    for(uint8_t i = 0; i < 6; ++i)                                                      //??KEYA?DATA0
    {
        thm3070_write_reg(DATA0, pKey[i]);
    }
    for(uint8_t i = 0; i < 4; ++i)                                                      //??UID?DATA0
    {
        thm3070_write_reg(DATA0, UID[i]);
    }
    
    thm3070_write_reg(RNGCON, 0x01);          // enable generation of  random number
    for (uint8_t i = 0; i < 4; ++i)
    {
        retry_times = 0;
        while (thm3070_read_reg(RNGSTS) != 1) // ready to read random number
        {
            if ((++retry_times) > MAX_WAIT_TIME)
            {
                return CODE_ERR;
            }
        }
        
        token[i] = thm3070_read_reg(RNGDATA);
    }
    thm3070_write_reg(DATA1, token[0]);
	thm3070_write_reg(DATA2, token[1]);
	thm3070_write_reg(DATA3, token[2]);
	thm3070_write_reg(DATA4, token[3]);
    thm3070_write_reg(RNGCON, 0x00);          // disable generation of  random number
    
    thm3070_write_reg(CRYPTO1_CTRL, 0x0C);    // Authentication begin
    
    retry_times = 0;    
    while (1)
    {
        auth_result = thm3070_read_reg(UART_STATE);
        if (auth_result)
        {
            break;
        }
        else if ((++retry_times) > MAX_WAIT_TIME)
        {
            return CODE_ERR;
        }
    }
    if ((auth_result & 0xEF) == 0)            // pass the authentication
    {
        thm3070_write_reg(CRYPTO1_CTRL, 0x08);// crypto1 enable
        thm3070_write_reg(STAT_CTRL, 0x01);   // data encroption enable
        return CODE_OK;
    }
    else if (auth_result & 0x80)
    {
        return 3;
    }
    else
    {
        return 4;
    }       
}
