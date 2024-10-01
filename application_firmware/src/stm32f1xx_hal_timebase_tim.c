
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

TIM_HandleTypeDef htim1;
void TIM1_IRQHandler(void);

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
  RCC_ClkInitTypeDef clkconfig;
  uint32_t uwTimclock = 0U;

  uint32_t uwPrescalerValue = 0U;
  uint32_t pFLatency;
  HAL_StatusTypeDef status = HAL_OK;

  __HAL_RCC_TIM1_CLK_ENABLE();

  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  uwTimclock = HAL_RCC_GetPCLK2Freq();

  uwPrescalerValue = (uint32_t)((uwTimclock / 1000000U) - 1U);

  htim1.Instance = TIM1;

  htim1.Init.Period = (1000000U / 1000U) - 1U;
  htim1.Init.Prescaler = uwPrescalerValue;
  htim1.Init.ClockDivision = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  status = HAL_TIM_Base_Init(&htim1);
  if (status == HAL_OK) {
    status = HAL_TIM_Base_Start_IT(&htim1);
    if (status == HAL_OK) {
      HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
      if (TickPriority < (1UL << __NVIC_PRIO_BITS)) {
        HAL_NVIC_SetPriority(TIM1_UP_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
      } else {
        status = HAL_ERROR;
      }
    }
  }

  return status;
}

void HAL_SuspendTick(void) { __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE); }

void HAL_ResumeTick(void) { __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE); }
