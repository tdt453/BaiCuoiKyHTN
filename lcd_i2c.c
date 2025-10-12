#include "lcd_i2c.h"

extern I2C_HandleTypeDef hi2c1;

void LCD_SendCommand(uint8_t command) {
    uint8_t data[4];
    uint8_t up = command & 0xF0;
    uint8_t lo = (command << 4) & 0xF0;
    data[0] = up | 0x0C; data[1] = up | 0x08;
    data[2] = lo | 0x0C; data[3] = lo | 0x08;
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data, 4, LCD_TIMEOUT);
}

void LCD_SendData(uint8_t data_byte) {
    uint8_t data[4];
    uint8_t up = data_byte & 0xF0;
    uint8_t lo = (data_byte << 4) & 0xF0;
    data[0] = up | 0x0D; data[1] = up | 0x09;
    data[2] = lo | 0x0D; data[3] = lo | 0x09;
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data, 4, LCD_TIMEOUT);
}

void LCD_Init(void) {
    HAL_Delay(50);
    LCD_SendCommand(0x30); HAL_Delay(5);
    LCD_SendCommand(0x30); HAL_Delay(1);
    LCD_SendCommand(0x30); HAL_Delay(10);
    LCD_SendCommand(0x20); HAL_Delay(10);
    LCD_SendCommand(0x28); HAL_Delay(1);
    LCD_SendCommand(0x0C); HAL_Delay(1);
    LCD_SendCommand(0x06); HAL_Delay(1);
    LCD_Clear();
}

void LCD_Clear(void) {
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0 ? 0x80 : 0xC0) + col;
    LCD_SendCommand(addr);
}

void LCD_Print(char *s) {
    while (*s) LCD_SendData((uint8_t)*s++);
}
