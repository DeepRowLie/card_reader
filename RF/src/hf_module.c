#include "hf_module.h"
#include "MFRC522.h"

#define MAX_FRM_LEN_ANTICOL   7
#define CT_FLAG             0x88
#define LEN_EACH_CASCADE      5

header g_card_tbl = NULL;

card_hf_uid g_hf_uid = {0};

uint8_t KEY_A_DEFAULT[KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t KEY_B_DEFAULT[KEY_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static uint8_t node_splitting_up(node);
static uint8_t BCC_check(uint8_t *, uint8_t);

uint8_t anti_collision_loop(node root, uint8_t cascade)
{   
    if (NODE_VALIDATE(root))
    {
        card_hf_uid *value         = root->value;
        uint8_t     *ptr_UID       = value->UID + (cascade - 1) * LEN_EACH_CASCADE;
        uint8_t      anti_col_frm[MAX_FRM_LEN_ANTICOL] = {0};
        uint8_t      tmp_valid_len = value->valid_len - (((cascade - 1) * LEN_EACH_CASCADE) << 3);
        uint8_t      TxLastBits    = tmp_valid_len % 8;
        uint8_t      TxBytes       = tmp_valid_len >> 3;
        uint8_t      TxBytesTotal  = TxBytes + (TxLastBits ? 1 : 0);
        uint8_t      status        = MI_OK;
        uint8_t      unLen         = 0;
    
        anti_col_frm[0] = PICC_ANTICOLL + ((cascade > 1) << (cascade - 1));
        anti_col_frm[1] = ((TxBytes + 2) << 4) + TxLastBits;
        memcpy(anti_col_frm + 2, ptr_UID, TxBytesTotal);
    
        Write_MFRC522(BitFramingReg, TxLastBits + (TxLastBits << 4));
    
        status = MFRC522_ToCard(PCD_TRANSCEIVE, anti_col_frm, TxBytesTotal + 2, anti_col_frm, (uint16_t *)&unLen);
    
        if (status == MI_OK)
        {
            unLen -= TxLastBits;
            
            for (uint8_t i = 0; i < ((unLen >> 3) + (unLen % 8 ? 1 : 0)); ++i)
            {
                ptr_UID[TxBytes + i] += anti_col_frm[i];
            }
            
            value->valid_len += unLen;
        
            if (BCC_check(ptr_UID, *(ptr_UID + LEN_EACH_CASCADE - 1)))
            {
                value->valid_len = 0;
                status = MI_ERR;
            }        
            else if (CT_FLAG == ptr_UID[LEN_EACH_CASCADE * (cascade - 1)])
            {
                status = anti_collision_loop(root, cascade + 1);
            }
            else
            {
                // achieve one complete UID, send HLTA and REQA
                MFRC522_Halt();                         // HLTA : change all PICC to IDLE
                MFRC522_Request(PICC_REQIDL, NULL);     // REQA : change all IDLE PICC to READY
            }
        }
        else if (status & MI_COL)
        {   
            uint8_t dummy_bits  = 0;
            uint8_t dummy_bytes = 0;
                
            unLen       = (Read_MFRC522(CollReg) & CollPos_MSK) - 1;
            dummy_bits  = TxLastBits + unLen;
            dummy_bytes = (dummy_bits >> 3) + (dummy_bits % 8 ? 1 : 0);
            *((uint32_t *)anti_col_frm) &= (1 << dummy_bits) - 1;
        
            for (uint8_t i = 0; i < dummy_bytes; ++i)
            {
                (value->UID)[TxBytes + i] += anti_col_frm[i];
            }
        
            value->valid_len += unLen;
        
            if (node_splitting_up(root))
            {
                return MI_ERR;
            }
        
            status = anti_collision_loop(root->l_child, cascade);
            status = anti_collision_loop(root->r_child, cascade);
        }
 
        return status;
    }
    else
    {
        return MI_OK;
    }    
}

void cb_hf_module_level_traverse(node root)
{
    if (NODE_VALIDATE(root))
    {
        NODE_PRINT(root);
    }
}

static uint8_t node_splitting_up(node root)
{
    node l_child  = NULL;
    node r_child  = NULL;
    card_hf_uid *value_l = NULL;
    card_hf_uid *value_r = NULL;
    card_hf_uid *value_base = root->value;
    uint8_t valid_bytes     = 0;
    uint8_t valid_bits      = 0;
    
    MALLOC_CHECK(l_child, sizeof(struct binary_tree), 1);
    MALLOC_CHECK(r_child, sizeof(struct binary_tree), 1);
    MALLOC_CHECK(l_child->value, sizeof(card_hf_uid), 1);
    MALLOC_CHECK(r_child->value, sizeof(card_hf_uid), 1);
    
    root->l_child = l_child;
    root->r_child = r_child;
    value_l = l_child->value;
    value_r = r_child->value;
    
    value_l->valid_len = value_r->valid_len = value_base->valid_len + 1;
    valid_bytes        = (value_base->valid_len) >> 3;
    valid_bits         = value_base->valid_len % 8;
    memcpy(value_l->UID, value_base->UID, valid_bytes);
    memcpy(value_r->UID, value_base->UID, valid_bytes);
    (value_l->UID)[valid_bytes] = (value_base->UID)[valid_bytes] & (~(uint8_t)(1 << valid_bits));
    (value_r->UID)[valid_bytes] = (value_base->UID)[valid_bytes] | (uint8_t)(1 << valid_bits);
    
    return MEM_OK;
}

static uint8_t BCC_check(uint8_t *data, uint8_t BCC)
{
    uint8_t status = MI_OK;
    uint8_t checksum = 0;

    for (uint8_t i = 0; i < 4 ; i++)
    {
        checksum ^= data[i];
    }
    
    if (checksum != BCC)
    {
        status = MI_ERR;
    }
    
    return status;
}
