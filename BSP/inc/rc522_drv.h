#ifndef __RC522_DRV_H
#define __RC522_DRV_H

#include "main.h"
#include "spi.h"

#define SPI_READ_CMD 1
#define SPI_WRITE_CMD 0
#define MAX_SPI_DATA_LEN 16
#define RC522_ADDR_BYTE_LEN 1
#define SPI_OK 0
#define SPI_ERR -1

//MF522 Command word
#define PCD_IDLE              0x00               //NO action; Cancel the current command
#define PCD_AUTHENT           0x0E               //Authentication Key
#define PCD_RECEIVE           0x08               //Receive Data
#define PCD_TRANSMIT          0x04               //Transmit data
#define PCD_TRANSCEIVE        0x0C               //Transmit and receive data,
#define PCD_RESETPHASE        0x0F               //Reset
#define PCD_CALCCRC           0x03               //CRC Calculate

// Mifare_One card command word
# define PICC_REQIDL          0x26               // find the antenna area does not enter hibernation
# define PICC_REQALL          0x52               // find all the cards antenna area
# define PICC_ANTICOLL        0x93               // anti-collision
# define PICC_SElECTTAG       0x93               // election card
# define PICC_AUTHENT1A       0x60               // authentication key A
# define PICC_AUTHENT1B       0x61               // authentication key B
# define PICC_READ            0x30               // Read Block
# define PICC_WRITE           0xA0               // write block
# define PICC_DECREMENT       0xC0               // debit
# define PICC_INCREMENT       0xC1               // recharge
# define PICC_RESTORE         0xC2               // transfer block data to the buffer
# define PICC_TRANSFER        0xB0               // save the data in the buffer
# define PICC_HALT            0x50               // Sleep


//And MF522 The error code is returned when communication
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

#define PRINTF_SPI_HANDLER(handler) \
        do \
        { \
            printf("\r\n--addr_byte:0x%X\r\n", *((uint8_t *)&(handler.addr_byte))); \
            printf("\r\n--direciton:0x%X\r\n", handler.addr_byte.direct); \
            printf("\r\n--reg:0x%X\r\n", handler.addr_byte.address); \
            printf("--tx_payload_len:0x%X\r\n", handler.tx_payload.len); \
            printf("--rx_payload_len:0x%X\r\n", handler.rx_payload.len); \
        }while(0)

typedef enum
{
    // Page 0: Command and status
    REV_00H = 0x00,
    CommandReg,
    ComlEnReg,
    DivlEnReg,
    ComIrqReg,
    DivIrqReg,
    ErrorReg,
    Status1Reg,
    Status2Reg,
    FIFODataReg,
    FIFOLevelReg,
    WaterLevelReg,
    ControlReg,
    BitFramingReg,
    CollReg,
    REV_0FH,
    // Page 1: Command
    REV_10H = 0x10,
    ModeReg,
    TxModeReg,
    RxModeReg,
    TxControlReg,
    TxASKReg,
    TxSelReg,
    RxSelReg,
    RxThresholdReg,
    DemodReg,
    REV_1AH,
    REV_1Bh,
    MfTxReg,
    MfRxReg,
    Rev_1EH,
    SerialSpeedReg,
    // Page 2: Configuration
    REV_20H = 0x20,
    CRCResultReg_MSB,
    CRCResultReg_LSB,
    REV_23H,
    ModWidthReg,
    REV_25H,
    RFCfgReg,
    GsNReg,
    CWGsPReg,
    ModGsPReg,
    TModeReg,
    TPrescalerReg,
    TReloadVal_Hi,
    TReloadVal_Lo,
    TCounterVal_Hi,
    TCounterVal_Lo,
    // Page 3: Test register
    REV_30H = 0x30,
    TestSel1Reg,
    TestSel2Reg,
    TestPinEnReg,
    TestPinValueReg,
    TestBusReg,
    AutoTestReg,
    VersionReg,
    AnalogTestReg,
    TestDAC1Reg,
    TestDAC2Reg,
    TestADCReg,
    REV_3CH,
    REV_3DH,
    REV_3EH,
    REV_3FH,
    RC522_REG_MAX,
}rc522_reg_addr;

typedef struct
{
    // little endian
    uint8_t rev_bit:1;
    uint8_t address:6;
    uint8_t direct:1;
}rc522_spi_addr_bytpe;

typedef struct
{
    uint8_t *pdata;
    uint8_t len;
}payload;

typedef struct
{
    SPI_HandleTypeDef *pSpihandler;

    rc522_spi_addr_bytpe addr_byte;

    payload tx_payload;
    payload rx_payload;
}rc522_spi_handler;

extern rc522_spi_handler g_rc522_spi;
extern uint8_t g_rx_complete_flag;

void rc522_drv_init(rc522_spi_handler *);
int8_t rc522_pkt_set(rc522_spi_handler *, rc522_reg_addr, uint8_t, uint8_t *, uint8_t, uint8_t);
int8_t rc522_spi_transmit(rc522_spi_handler *);

// function definitions
void Write_MFRC522(uint8_t, uint8_t);
uint8_t Read_MFRC522(uint8_t);
void SetBitMask(uint8_t, uint8_t);
void ClearBitMask(uint8_t, uint8_t);
void AntennaOn();
void AntennaOff();
void MFRC522_Reset();
void MFRC522_Init();
uint8_t MFRC522_Request(uint8_t, uint8_t*);
uint8_t MFRC522_ToCard(uint8_t, uint8_t*, uint8_t, uint8_t*, uint*);
uint8_t MFRC522_Anticoll(uint8_t*);
void CalulateCRC(uint8_t*, uint8_t, uint8_t*);
uint8_t MFRC522_SelectTag(uint8_t*);
uint8_t MFRC522_Auth(uint8_t, uint8_t, uint8_t*, uint8_t*);
uint8_t MFRC522_Read(uint8_t, uint8_t*);
uint8_t MFRC522_Write(uint8_t, uint8_t*);
void MFRC522_Halt();
void MFRC522_StopCrypto1(void);

#endif  /*__RC522_DRV_H*/
