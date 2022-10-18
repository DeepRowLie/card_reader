#include "fputc.h"

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart2,(uint8_t *)&ch,1,0xFFFF);
	return ch;
}
