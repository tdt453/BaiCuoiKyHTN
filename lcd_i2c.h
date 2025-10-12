#ifndef __LCD_I2C_H
#define __LCD_I2C_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define LCD_ADDR      (0x4E)
#define LCD_TIMEOUT   100

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(char *s);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);

#endif
