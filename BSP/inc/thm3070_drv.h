#ifndef __THM3070_DRV_H
#define __THM3070_DRV_H

#include "main.h"
#include "spi.h"

// custom MACRO
#define MAX_DATA_LEN    256
#define CODE_ERR        1
#define CODE_OK         0
#define MAX_WAIT_TIME  0x4400

// BIT mask
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

// operation mode
#define ISO_IEC14443_A 0x01   // protocol ISO/IEC TYPE A
#define ISO_IEC14443_B 0x00   // protocol ISO/IEC TYPE B
#define ISO_IEC15693   0x02   // protocol ISO/IEC 15693

// FRM format select
#define SHORT_FRM     1
#define BIT_ORIENTED  2
#define STD_FRM       3
 
// THM3070 register RSTAT 
#define THM_RSTST_IRQ     BIT7
#define THM_RSTST_CERR    BIT6
#define THM_RSTST_PERR    BIT5
#define THM_RSTST_FERR    BIT4
#define THM_RSTST_DATOVER BIT3
#define THM_RSTST_TMROVER BIT2
#define THM_RSTST_CRCERR  BIT1
#define THM_RSTST_FEND    BIT0

// protocol ISO/IEC TYPE A command code
#define REQA 0x26
#define SEL_C1 0x93
//#define SEL_C2 SEL_C1+2
//#define SEL_C3 SEL_C1+4
#define ATQA_LEN 2

// mifare specified auth process
#define auth_withKeyA 0x60
#define auth_withKeyB 0x61 

/**
 *brief : enum thm3070_reg_addr records the address of dest register
 */
typedef enum
{
    DATA,
    PSEL,
    FCONB,
    EGT,
    CRCSEL,
    RSTAT,
    SCON,
    INTCON,
    RSCH,
    RSCL,
    CRCH,
    CRCL,
    TMRH,
    TMRL,
    BITPOS,
    SMOD = 0x10,
    PWTH,
    // unknown register begin
    STAT_CTRL,     // 0x12
    FM_CTRL,       // 0x13
    UART_STATE,    // 0x14
    CRYPTO1_CTRL,  // 0x15
    DATA0,         // 0x16
    DATA1,         // 0x17
    DATA2,         // 0x18
    DATA3,         // 0x19
    DATA4,         // 0x1A
    COL_STAT,      // 0x1B
    SND_CTRL,      // 0x1C
    // unknown register end
    EMVEN = 0x20,
    FWIHIGH,
    FWIMID,
    FWILOW,
    AFDTOFFSET,
    EMVERR,
    TXFIN,
    TR0MINH = 0x2E,
    TR0MINL,
    RNGCON,
    RNGSTS,
    RNGDATA,
    TR1MINH,
    TR1MINL,
    TR1MAXH,
    TR1MAXL,
    TXCON = 0x40,
    TXDP1,
    TXDP0,
    TXDN1,
    TXDN0,
    RXCON,
    ADDR_MAX,
}thm3070_reg_addr;

extern uint8_t Key_A[6];
extern uint8_t Key_B[6];

/***************************************************
************** THM3070 DRV interface ***************
***************************************************/
// register I/O interface
void thm3070_write_reg(thm3070_reg_addr addr, uint8_t data);
uint8_t thm3070_read_reg(thm3070_reg_addr addr);
void SetBitMask(uint8_t reg, uint8_t mask);
void ClearBitMask(uint8_t reg, uint8_t mask);

// THM3070 DRV initialization interface
uint8_t thm3070_Init(void);
void thm3070_AntennaOn(void);
void thm3070_AntennaOff(void);
void thm3070_reset(void);

// operation mode interface
uint8_t thm3070_setMode_mifare(void);
uint8_t thm3070_setMode_14443A(void);
uint8_t thm3070_setMode_14443B(void);
uint8_t thm3070_setMode_15693(void);
uint8_t thm3070_setFWT(uint32_t fwt);

// wireless communication interface
uint8_t thm3070_request(uint8_t reqMode);
uint8_t thm3070_Anticoll(uint8_t *serNum, uint8_t *serNum_len);
uint8_t thm3070_selectTag(uint8_t *serNum, uint8_t serNum_len);
uint8_t thm3070_mifareAuth(uint8_t auth_keyType, uint8_t blockNum, uint8_t *UID);

#endif  /*__THM3070_DRV_H*/
