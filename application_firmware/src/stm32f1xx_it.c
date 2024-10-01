
#include "stm32f1xx_it.h"
#include "main.h"

extern DMA_HandleTypeDef hdma_adc1;
extern ADC_HandleTypeDef hadc1;
extern RTC_HandleTypeDef hrtc;
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

void RTC_IRQHandler(void) { HAL_RTCEx_RTCIRQHandler(&hrtc); }

void RCC_IRQHandler(void) {}

void DMA1_Channel1_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_adc1); }

void ADC1_2_IRQHandler(void) { HAL_ADC_IRQHandler(&hadc1); }

void EXTI1_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1); }

void TIM1_UP_IRQHandler(void) { HAL_TIM_IRQHandler(&htim1); }

void USART3_IRQHandler(void) { HAL_UART_IRQHandler(&huart3); }
