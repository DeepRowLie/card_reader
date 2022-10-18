#ifndef __USB_CUSTOM_H
#define __USB_CUSTOM_H

#include "main.h"
#include "tim.h"

#define MAX_USB_BUF 128
#define PREAMBLE 0xAA55
#define UP_STREAM 0x01
#define DOWN_STREAM 0x00
#define LEN_MIN 3 //the minimun value of packet's len field
#define PACKET_LEN_MIN 6 // the minimun value of packet
#define HEAD_OFFSET 2
#define CMD_SUCCESS 0x00
#define CMD_FAIL 0x01
#define ACK 1
#define NACK 0

#define CUSTOM_USB_MESSAGE_CONSTRUCTOR(DEST, SRC, PAYLOAD) \
        do\
        {\
            DEST.head   = SRC.head;\
            DEST.len    = PAYLOAD.data_len + LEN_MIN;\
            DEST.cmd    = SRC.cmd;\
            DEST.subcmd = UP_STREAM;\
            DEST.payload.pdata    = PAYLOAD.pdata;\
            DEST.payload.data_len = PAYLOAD.data_len;\
            DEST.valid  = SRC.valid;\
        }while(0)
        
#define PRINTF_CUSTOM_USB_MSG(MSG) \
        do\
        {\
            printf("\r\n--head:0x%X\r\n", MSG.head);\
            printf("--len:0x%X\r\n", MSG.len);\
            printf("--cmd:0x%X\r\n", MSG.cmd);\
            printf("--subcmd:0x%X\r\n", MSG.subcmd);\
            printf("--data_len:0x%X\r\n", MSG.payload.data_len);\
            printf("--check:0x%X\r\n", MSG.check);\
        }while(0)

#define REGISTER_COMMAND(CMD_TABLE, CMD_ID, DESCRIPTION, ACT_HANDLER) \
        do\
        {\
            CMD_TABLE[CMD_ID].index = CMD_ID;\
            CMD_TABLE[CMD_ID].description = DESCRIPTION;\
            CMD_TABLE[CMD_ID].func_handler = ACT_HANDLER;\
        }while(0)
        
typedef enum
{
    CONNECT_CMD,
    READ_CMD,
    WRITE_CMD,
    BUZZER_TEST,
    LED_TEST,
    PASM_TEST,
    SET_TRANS_POWER,
    READ_TRANS_POWER,
    MAX_CMD,
}cmd_id;
        
typedef struct
{
    uint8_t *pdata;
    uint16_t data_len;
}msg_data;

typedef struct
{
    uint16_t head;
    uint8_t  len;
    uint8_t  cmd;
    uint8_t  subcmd;
    msg_data payload;
    uint8_t  check;
    uint8_t  valid;
}custom_usb_message;

typedef uint8_t (*cmd_function)(msg_data *, msg_data *);

typedef struct
{
    cmd_id index;
    uint8_t *description;
    cmd_function func_handler;
}command_vector;

extern custom_usb_message g_usb_message;
extern uint8_t g_usb_tx_buf[MAX_USB_BUF];
extern command_vector g_cmd_table[MAX_CMD];

void usd_custom_init(void);
void parse_packet(uint8_t *Buf, uint32_t Len, custom_usb_message *message);
uint16_t assemble_packet(custom_usb_message *message, uint8_t *tx_buf);
uint8_t checksum(custom_usb_message *message);
uint8_t validate(custom_usb_message *message);

#endif  /*__USB_CUSTOM_H*/
