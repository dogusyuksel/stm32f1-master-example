#include "main.h"
#include "FreeRTOS.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "printf.h"
#include "rtc.h"
#include "task.h"
#include "uavcan.h"
#include "usart.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ADC_DMA_BUFFER_LEN 16

#define CURRENT_YEAR 2024

uint16_t adc_dma_buffer[ADC_DMA_BUFFER_LEN];
uint16_t adc_result_in_mv = 0;

void SystemClock_Config(void);
void uart_log(const char *format, ...);
void led_control(uint8_t led_code, led_state led_state);

void uart_log(const char *format, ...) {
  va_list arguments;
  char buffer[128] = {0};

  va_start(arguments, format);
  vsnprintf_(buffer, sizeof(buffer), format, arguments);
  va_end(arguments);

  if (strlen(buffer) == 0) {
    return;
  }

  HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen((char *)buffer),
                    strlen((char *)buffer));
}

void led_control(uint8_t led_code, led_state led_state) {
  uint8_t i = 0;

  if (led_state != LED_TOGGLE) {
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, (GPIO_PinState)LED_RESET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,
                      (GPIO_PinState)LED_RESET);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin,
                      (GPIO_PinState)LED_RESET);
  }

  for (i = 1; i < 8; i++) {
    uint8_t shifted_value = LED_OFF;

    shifted_value = (shifted_value << i);

    if ((led_code & shifted_value) == LED_RED) {
      if (led_state == LED_TOGGLE) {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
      } else {
        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,
                          (GPIO_PinState)led_state);
      }
    }
    if ((led_code & shifted_value) == LED_GREEN) {
      if (led_state == LED_TOGGLE) {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
      } else {
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,
                          (GPIO_PinState)led_state);
      }
    }
    if ((led_code & shifted_value) == LED_BLUE) {
      if (led_state == LED_TOGGLE) {
        HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
      } else {
        HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin,
                          (GPIO_PinState)led_state);
      }
    }
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_1) {
    // uart_log("PPS generated\n");
    led_control(LED_RED, LED_TOGGLE);
  }
  portYIELD_FROM_ISR(pdTRUE);
}

static void set_rtc(uint16_t Year, uint8_t Month, uint8_t Date, uint8_t WeekDay,
                    uint8_t Hours, uint8_t Minutes, uint8_t Seconds) {
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  sTime.Hours = Hours;
  sTime.Minutes = Minutes;
  sTime.Seconds = Seconds;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
    Error_Handler();
  }
  sDate.WeekDay = WeekDay;
  sDate.Month = Month;
  sDate.Date = Date;
  if (Year >= CURRENT_YEAR) {
    sDate.Year = (uint8_t)(Year - CURRENT_YEAR);
  } else {
    sDate.Year = (uint8_t)Year;
  }
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
    Error_Handler();
  }
}

static unsigned int formatter(unsigned int number) {
  char buffer[5] = {0};
  char *rest = NULL;

  snprintf(buffer, sizeof(buffer), "%x", number);

  return strtoul(buffer, &rest, 10);
}

static void print_rtc(void) {
  RTC_DateTypeDef sdatestructget;
  RTC_TimeTypeDef stimestructget;

  uint8_t second = 0;
  uint8_t minute = 0;
  uint8_t hour = 0;
  uint8_t day = 0;
  uint8_t month = 0;
  uint16_t year = 0;

  HAL_RTC_GetTime(&hrtc, &stimestructget, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(&hrtc, &sdatestructget, RTC_FORMAT_BCD);

  second = stimestructget.Seconds;
  second = formatter(second);
  minute = stimestructget.Minutes;
  minute = formatter(minute);
  hour = stimestructget.Hours;
  hour = formatter(hour);
  day = sdatestructget.Date;
  day = formatter(day);
  month = sdatestructget.Month;
  month = formatter(month);
  year = sdatestructget.Year + CURRENT_YEAR;

  uart_log("%.02u-%.02u-%.04u %.02u:%.02u:%.02u\n", day, month, year, hour,
           minute, second);
}

void generic_task(void *data) {
  (void)data;

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(5000));

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint16_t adc_result = HAL_ADC_GetValue(&hadc1);
    adc_result_in_mv = ((3300 * adc_result) / 4095);
    adc_result_in_mv = (adc_result_in_mv * 5.5) / 3.3;

    uart_log("adc_result_in_mv: %u\n", adc_result_in_mv);
    led_control(LED_BLUE, LED_TOGGLE);

    print_rtc();

    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);

    taskYIELD();
  }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  (void)hadc;
  led_control(LED_BLUE, LED_TOGGLE);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  (void)hadc;
  uint32_t i = 0;
  uint64_t sum = 0;
  for (i = 0; i < sizeof(adc_dma_buffer) / sizeof(adc_dma_buffer[0]); i++) {
    sum += adc_dma_buffer[i];
  }

  sum = sum / (sizeof(adc_dma_buffer) / sizeof(adc_dma_buffer[0]));

  adc_result_in_mv = ((3300 * sum) / 4095);
}

void canardtask(void *data) {
  (void)data;
  uavcanInit();

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(500));
    sendCanard();
    receiveCanard();
    spinCanard();
    taskYIELD();
  }
}

int main(void) {

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_USART3_UART_Init();

  uart_log("application started!\n");

  led_control(LED_RED | LED_GREEN | LED_BLUE, LED_RESET);
  set_rtc(CURRENT_YEAR, RTC_MONTH_OCTOBER, 0x01, RTC_WEEKDAY_TUESDAY, 0x13,
          0x34, 0x40);

  xTaskCreate(generic_task, "generic_task", 512, NULL, 2, NULL);
  xTaskCreate(canardtask, "canardtask", 1024, NULL, 4, NULL);
  vTaskStartScheduler();

  while (1) {
  }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
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
