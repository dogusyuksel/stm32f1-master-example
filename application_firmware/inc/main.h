
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define LED_OFF 0x01
#define LED_RED 0x02
#define LED_GREEN 0x04
#define LED_BLUE 0x08
typedef enum {
  LED_SET = GPIO_PIN_RESET,
  LED_RESET = GPIO_PIN_SET,
  LED_TOGGLE,
  LED_STATE_COUNT
} led_state;

void Error_Handler(void);

#define ANALOG_EXTERNAL_Pin GPIO_PIN_5
#define ANALOG_EXTERNAL_GPIO_Port GPIOA
#define LED_RED_Pin GPIO_PIN_13
#define LED_RED_GPIO_Port GPIOB
#define LED_GREEN_Pin GPIO_PIN_14
#define LED_GREEN_GPIO_Port GPIOB
#define LED_BLUE_Pin GPIO_PIN_15
#define LED_BLUE_GPIO_Port GPIOB

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
