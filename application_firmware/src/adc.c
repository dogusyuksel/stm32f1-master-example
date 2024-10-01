#include "adc.h"
#include "FreeRTOS.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

void MX_ADC1_Init(void) {

  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  sConfig.Rank = ADC_REGULAR_RANK_2;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (adcHandle->Instance == ADC1) {

    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = ANALOG_EXTERNAL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(ANALOG_EXTERNAL_GPIO_Port, &GPIO_InitStruct);

    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);

    HAL_NVIC_SetPriority(ADC1_2_IRQn,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    NVIC_SetPriority(ADC1_2_IRQn,
                     configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle) {

  if (adcHandle->Instance == ADC1) {

    __HAL_RCC_ADC1_CLK_DISABLE();

    HAL_GPIO_DeInit(ANALOG_EXTERNAL_GPIO_Port, ANALOG_EXTERNAL_Pin);

    HAL_DMA_DeInit(adcHandle->DMA_Handle);

    HAL_NVIC_DisableIRQ(ADC1_2_IRQn);
  }
}
