#include "rc522_drv.h"

rc522_spi_handler g_rc522_spi = {0};
uint8_t g_rx_complete_flag = FALSE;
static uint8_t sg_tx_payload[MAX_SPI_DATA_LEN] = {0};
static uint8_t sg_tx_buf[MAX_SPI_DATA_LEN] = {0};
static uint8_t sg_rx_buf[MAX_SPI_DATA_LEN] = {0};

void rc522_drv_init(rc522_spi_handler *phandler)
{
    phandler->pSpihandler = &hspi1;
    phandler->rx_payload.pdata = sg_rx_buf;
}

int8_t rc522_pkt_set(rc522_spi_handler *phandler, rc522_reg_addr reg, uint8_t direction, uint8_t *ptxdata, uint8_t tx_data_len, uint8_t rx_data_len)
{
    if (reg >= RC522_REG_MAX
        || (direction == SPI_WRITE_CMD && (!ptxdata || 0 == tx_data_len))
        || (direction == SPI_READ_CMD && 0 == rx_data_len))
    {
        return SPI_ERR;
    }
    else
    {
        rc522_spi_addr_bytpe *paddr_byte = &(phandler->addr_byte);
        payload *ptx_payload = &(phandler->tx_payload);
        payload *prx_payload = &(phandler->rx_payload);

        paddr_byte->address = reg;
        paddr_byte->direct = direction;
        ptx_payload->pdata = ptxdata;
        ptx_payload->len = tx_data_len;
        prx_payload->pdata = NULL;
        prx_payload->len = rx_data_len;

        return SPI_OK;
    }
}

int8_t rc522_spi_transmit(rc522_spi_handler *phandler)
{
    uint8_t *ptx_buf = sg_tx_buf;
    rc522_spi_addr_bytpe *paddr_byte = &(phandler->addr_byte);
    uint8_t pkt_len = RC522_ADDR_BYTE_LEN;
    HAL_StatusTypeDef ret = HAL_OK;

    *(ptx_buf++) = *((uint8_t *)paddr_byte);
    if (paddr_byte->direct)
    {
        // read register
        g_rx_complete_flag = FALSE;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        ret = HAL_SPI_TransmitReceive_DMA(phandler->pSpihandler, sg_tx_buf, sg_rx_buf, pkt_len + phandler->rx_payload.len);
    }
    else
    {
        // write register
        payload *ptx_payload = &(phandler->tx_payload);
        uint8_t tx_data_len = ptx_payload->len;

        memcpy(ptx_buf, ptx_payload->pdata, tx_data_len);
        pkt_len += tx_data_len;

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        ret = HAL_SPI_Transmit_DMA(phandler->pSpihandler, sg_tx_buf, pkt_len);
    }

    return SPI_OK;
}

void Write_MFRC522(uint8_t reg, uint8_t val)
{
    sg_tx_payload[0] = val;
    
    rc522_pkt_set(&g_rc522_spi, reg, SPI_WRITE_CMD, sg_tx_payload, 1, 0);
    rc522_spi_transmit(&g_rc522_spi);
}

uint8_t Read_MFRC522(uint8_t reg)
{
    uint8_t rx_data = 0;
    
    rc522_pkt_set(&g_rc522_spi, reg, SPI_READ_CMD, NULL, 0, 1);
    rc522_spi_transmit(&g_rc522_spi);
    
    while(!g_rx_complete_flag);
    
    return rx_data = *(g_rc522_spi.rx_payload.pdata);
}

void SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);  // set bit mask
}

void ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));  // clear bit mask
}

void AntennaOn(void)
{
  SetBitMask(TxControlReg, 0x03);
}

void AntennaOff(void)
{
  ClearBitMask(TxControlReg, 0x03);
}

void MFRC522_Reset(void)
{
  Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

void MFRC522_Init(void)
{
//  MSS_GPIO_set_output( MSS_GPIO_1, 1 );
  MFRC522_Reset();

  // Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
  Write_MFRC522(TModeReg, 0x80); // 0x8D);      // Tauto=1; f(Timer) = 6.78MHz/TPreScaler
  Write_MFRC522(TPrescalerReg, 0xA9); //0x34); // TModeReg[3..0] + TPrescalerReg
  Write_MFRC522(TReloadVal_Lo, 0x03); //30);
  Write_MFRC522(TReloadVal_Hi, 0xE8); //0);
  Write_MFRC522(TxASKReg, 0x40);     // force 100% ASK modulation
  Write_MFRC522(ModeReg, 0x3D);       // CRC Initial value 0x6363

  // turn antenna on
  AntennaOn();
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType)
{
  uint8_t status;
  uint backBits; // The received data bits

  Write_MFRC522(BitFramingReg, 0x07);   // TxLastBists = BitFramingReg[2..0]

  TagType[0] = reqMode;

  status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
  if ((status != MI_OK) || (backBits != 0x10)) {
    status = MI_ERR;
  }

  return status;
}

