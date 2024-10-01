#include "usart.h"
#include "FreeRTOS.h"

UART_HandleTypeDef huart3;

void MX_USART3_UART_Init(void) {

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART3) {

    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // this configuration is needed to use "..FromISR" like RTOS functions
    HAL_NVIC_SetPriority(USART3_IRQn,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_SetPriority(USART3_IRQn,
                     configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {

  if (uartHandle->Instance == USART3) {

    __HAL_RCC_USART3_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);

    HAL_NVIC_DisableIRQ(USART3_IRQn);
  }
}
