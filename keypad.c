#include "keypad.h"

#define ROWS 4
#define COLS 4

// Ð?nh nghia GPIO
#define KEYPAD_ROW_PINS {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3}
#define KEYPAD_COL_PINS {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7}
#define KEYPAD_ROW_PORT GPIOA
#define KEYPAD_COL_PORT GPIOA

char keypad_keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

static GPIO_TypeDef* rowPorts[ROWS] = {KEYPAD_ROW_PORT, KEYPAD_ROW_PORT, KEYPAD_ROW_PORT, KEYPAD_ROW_PORT};
static GPIO_TypeDef* colPorts[COLS] = {KEYPAD_COL_PORT, KEYPAD_COL_PORT, KEYPAD_COL_PORT, KEYPAD_COL_PORT};
static uint16_t rowPins[ROWS] = KEYPAD_ROW_PINS;
static uint16_t colPins[COLS] = KEYPAD_COL_PINS;

void Keypad_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Rows: Output
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);

    // Cols: Input Pull-up
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

char read_keypad(void) {
    for (int r = 0; r < 4; r++) {
        for (int i = 0; i < 4; i++) HAL_GPIO_WritePin(rowPorts[i], rowPins[i], GPIO_PIN_SET);
        HAL_GPIO_WritePin(rowPorts[r], rowPins[r], GPIO_PIN_RESET);
        HAL_Delay(1);
        for (int c = 0; c < 4; c++) {
            if (HAL_GPIO_ReadPin(colPorts[c], colPins[c]) == GPIO_PIN_RESET) {
                HAL_Delay(30);
                if (HAL_GPIO_ReadPin(colPorts[c], colPins[c]) == GPIO_PIN_RESET) {
                    while (HAL_GPIO_ReadPin(colPorts[c], colPins[c]) == GPIO_PIN_RESET);
                    for (int i = 0; i < 4; i++) HAL_GPIO_WritePin(rowPorts[i], rowPins[i], GPIO_PIN_SET);
                    return keypad_keys[r][c];
                }
            }
        }
    }
    return 0;
}
