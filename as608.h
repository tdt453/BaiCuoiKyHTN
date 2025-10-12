#ifndef __AS608_H
#define __AS608_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_ID_SELECT 50
#define FP_UART_TIMEOUT 2000

bool FP_GetImage(void);
bool FP_Image2Tz(uint8_t bufferID);
bool FP_RegModel(void);
bool FP_Store(uint16_t id);
int16_t FP_Search(void);
bool FP_DeleteAll(void);

#endif
