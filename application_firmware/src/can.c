#include "can.h"
#include "FreeRTOS.h"

CAN_HandleTypeDef hcan;

void MX_CAN_Init(void) {

  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 16;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (canHandle->Instance == CAN1) {

    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle) {

  if (canHandle->Instance == CAN1) {

    __HAL_RCC_CAN1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
  }
}
