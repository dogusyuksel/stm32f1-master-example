
#include "dma.h"
#include "FreeRTOS.h"

void MX_DMA_Init(void) {

  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn,
                       configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1,
                       configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
  NVIC_SetPriority(DMA1_Channel1_IRQn,
                   configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}
