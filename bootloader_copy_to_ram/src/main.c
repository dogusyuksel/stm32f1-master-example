#include "main.h"
#include "gpio.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint32_t __ram_fwarea_begin;
extern uint32_t __ram_fwarea_end;
#define RAM_FWAREA_BEGIN ((uint32_t)(&__ram_fwarea_begin))
#define RAM_FWAREA_END ((uint32_t)(&__ram_fwarea_end))

extern uint32_t __flash_fwarea_begin;
extern uint32_t __flash_fwarea_end;
#define FLASH_FWAREA_BEGIN ((uint32_t)(&__flash_fwarea_begin))
#define FLASH_FWAREA_END ((uint32_t)(&__flash_fwarea_end))

void SystemClock_Config(void);
typedef void (*pFunction)(void);

uint8_t buffer[128] = {0};
uint8_t buffer_counter = 0;
uint8_t read_byte = 0;
uint8_t address_read = 0;

void myjump(uint32_t address) {
  uint32_t jump_address = *(uint32_t *)(address + 4);
  pFunction jump_function = (pFunction)jump_address;

  // De initialize RCC, HAL and disable IRQs
  HAL_RCC_DeInit();
  HAL_DeInit();
  __disable_irq();

  // Reset the SysTick timer
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  // Set Vector tab offset
  SCB->VTOR = address;
  // Update the Master Stack Pointer
  __set_MSP(*(uint32_t *)address);
  // Activate (privileged) thread mode
  __set_CONTROL(0);
  // Enable IRQs before we jump into the firmware
  __enable_irq();

  jump_function();
}

void copyF2R(uint32_t address) {
  uint32_t *pSrc;
  uint32_t *pDst;

  pSrc = (uint32_t *)address;
  pDst = (uint32_t *)RAM_FWAREA_BEGIN;
  /* the size of app is about 32KB with newlib */
  while (pDst < (uint32_t *)RAM_FWAREA_END) {
    *pDst++ = *pSrc++;
  }

  char message[128] = {0};
  snprintf(message, sizeof(message), "copy done\n");
  HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen((char *)message), 500);

  myjump(RAM_FWAREA_BEGIN);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart3) {
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&read_byte, 1);
    buffer[buffer_counter++] = read_byte;
    if (read_byte == (uint8_t)'\n') {
      address_read = 1;
    }
    if (buffer_counter >= sizeof(buffer)) {
      buffer_counter = 0;
    }
  }
}

int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART3_UART_Init();

  HAL_UART_Receive_IT(&huart3, (uint8_t *)&read_byte, 1);

#ifdef COPY_TO_RAM
  HAL_UART_Transmit(&huart3, (uint8_t *)"It is RAM Bootloader\n", 21, 500);
#else
  HAL_UART_Transmit(&huart3, (uint8_t *)"It is FLASH Bootloader\n", 23, 500);
#endif

  while (1) {
    HAL_UART_Transmit(&huart3, (uint8_t *)"waiting address\n", 16, 500);
    HAL_Delay(1000);

    if (address_read) {
      char message[164] = {0};
#ifdef COPY_TO_RAM
      char *rest;
      uint32_t address = strtoul((char *)buffer, &rest, 16);
      snprintf(message, sizeof(message), "jump address: %s (%lx)\n",
               (char *)buffer, address);
#else
      snprintf(message, sizeof(message), "jump address: %lx\n",
               FLASH_FWAREA_BEGIN);
#endif
      HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen((char *)message),
                        500);
#ifdef COPY_TO_RAM
      copyF2R(address);
#else
      myjump(FLASH_FWAREA_BEGIN);
#endif
    }
  }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
}

void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif /* USE_FULL_ASSERT */
