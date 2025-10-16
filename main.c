/* main.c - Fingerprint Door Lock (split into modules)
   Ghi chu: chu thich tieng Viet khong dau
*/

#include "stm32f1xx_hal.h"
#include "lcd_i2c.h"
#include "keypad.h"
#include "as608.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ------------------- Handles ------------------- */
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
#define ADMIN_PASS "1234"
/* ------------------- States ------------------- */
typedef enum {
    STATE_IDLE = 0,
    STATE_ADD_FINGER,
    STATE_ADD_STEP1,
    STATE_ADD_STEP2,
    STATE_ADD_SAVE,
    STATE_VERIFY,
    STATE_DELETE_ALL,
    STATE_EXIT
} SystemState;

/* ------------------- Global Variables ------------------- */
SystemState current_state = STATE_IDLE;
uint16_t enroll_id = 1;

/* ------------------- Function Prototypes ------------------- */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);

/* ------------------- Main Program ------------------- */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    Keypad_Init();
    LCD_Init();

    LCD_Clear();
    LCD_SetCursor(0,0); LCD_Print("Fingerprint Lock");
    LCD_SetCursor(1,0); LCD_Print("System Starting...");
    HAL_Delay(2000);
    LCD_Clear();

    while (1)
    {
        switch (current_state)
        {
            /* ----------- MAIN MENU ----------- */
            case STATE_IDLE:
            {
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("1:Add 2:DelAll");
                LCD_SetCursor(1,0); LCD_Print("3:Exit");
                char key = 0;
                while (key == 0) {
                    key = read_keypad();
                    HAL_Delay(100);
                }

                if (key == '1')
                    current_state = STATE_ADD_FINGER;
                else if (key == '2')
                    current_state = STATE_DELETE_ALL;
                else if (key == '3')
                    current_state = STATE_EXIT;
                else if (key == 'B')
                    current_state = STATE_VERIFY;
                break;
            }

            /* ----------- ADD FINGER MENU ----------- */
            case STATE_ADD_FINGER:
            {
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("Select ID:");
                char buf[16];
                sprintf(buf, "%d", enroll_id);
                LCD_SetCursor(1,0); LCD_Print(buf);

                char key = 0;
                while (key == 0) {
                    key = read_keypad();
                    HAL_Delay(100);
                }

                if (key == 'A') {
                    if (enroll_id < MAX_ID_SELECT)
                        enroll_id++;
                } else if (key == 'C') {
                    if (enroll_id > 1)
                        enroll_id--;
                } else if (key == 'B') {
                    current_state = STATE_ADD_STEP1;
                } else if (key == 'D') {
                    current_state = STATE_IDLE;
                }
                break;
            }

            /* ----------- ADD STEP 1 ----------- */
            case STATE_ADD_STEP1:
            {
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("Place finger #1");
                while (!FP_GetImage());
                FP_Image2Tz(1);
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("Remove finger");
                HAL_Delay(2000);
                current_state = STATE_ADD_STEP2;
                break;
            }

            /* ----------- ADD STEP 2 ----------- */
            case STATE_ADD_STEP2:
            {
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("Place finger #2");
                while (!FP_GetImage());
                FP_Image2Tz(2);

                if (FP_RegModel())
                    current_state = STATE_ADD_SAVE;
                else {
                    LCD_Clear();
                    LCD_SetCursor(0,0); LCD_Print("Match failed");
                    HAL_Delay(1500);
                    current_state = STATE_IDLE;
                }
                break;
            }

            /* ----------- ADD SAVE ----------- */
            case STATE_ADD_SAVE:
            {
                if (FP_Store(enroll_id)) {
                    LCD_Clear();
                    LCD_SetCursor(0,0); LCD_Print("Saved ID:");
                    char buf[8];
                    sprintf(buf, "%d", enroll_id);
                    LCD_SetCursor(1,0); LCD_Print(buf);
                } else {
                    LCD_Clear();
                    LCD_SetCursor(0,0); LCD_Print("Save failed!");
                }
                HAL_Delay(2000);
                current_state = STATE_IDLE;
                break;
            }

            /* ----------- VERIFY ----------- */
            case STATE_VERIFY:
            {
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("Place finger...");
                while (!FP_GetImage());
                FP_Image2Tz(1);
                int16_t id = FP_Search();

                LCD_Clear();
                if (id >= 0) {
                    LCD_SetCursor(0,0); LCD_Print("Access Granted");
                    LCD_SetCursor(1,0);
                    char buf[16];
                    sprintf(buf, "ID:%d", id);
                    LCD_Print(buf);

                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
                    HAL_Delay(3000);
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                } else {
                    LCD_SetCursor(0,0); LCD_Print("Access Denied");
                    HAL_Delay(2000);
                }
                current_state = STATE_IDLE;
                break;
            }

            /* ----------- DELETE ALL ----------- */
            case STATE_DELETE_ALL:
            {
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("Delete all IDs?");
                LCD_SetCursor(1,0); LCD_Print("A:Yes  D:No");

                char key = 0;
                while (key == 0) key = read_keypad();

                if (key == 'A') {
                    LCD_Clear();
                    LCD_SetCursor(0,0); LCD_Print("Deleting...");
                    if (FP_DeleteAll()) {
                        LCD_Clear();
                        LCD_SetCursor(0,0); LCD_Print("All deleted");
                    } else {
                        LCD_Clear();
                        LCD_SetCursor(0,0); LCD_Print("Delete failed");
                    }
                    HAL_Delay(2000);
                }
                current_state = STATE_IDLE;
                break;
            }

            /* ----------- EXIT ----------- */
            case STATE_EXIT:
            {
                LCD_Clear();
                LCD_SetCursor(0,0); LCD_Print("System stopped");
                HAL_Delay(2000);
                while (1);
            }
        }
    }
}

/* ------------------- Clock Config ------------------- */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* ------------------- I2C Init ------------------- */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

/* ------------------- UART Init ------------------- */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 57600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart1);
}

/* ------------------- GPIO Init ------------------- */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

