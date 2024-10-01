
#include "stm32f1xx_it.h"
#include "main.h"

extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim1;

void NMI_Handler(void) {

  while (1) {
  }
}

void HardFault_Handler(void) {

  while (1) {
  }
}

void MemManage_Handler(void) {

  while (1) {
  }
}

void BusFault_Handler(void) {

  while (1) {
  }
}

void UsageFault_Handler(void) {

  while (1) {
  }
}

void DebugMon_Handler(void) {}

void TIM1_UP_IRQHandler(void) { HAL_TIM_IRQHandler(&htim1); }

void USART3_IRQHandler(void) { HAL_UART_IRQHandler(&huart3); }
