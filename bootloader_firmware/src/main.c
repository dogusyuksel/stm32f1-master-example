#include "main.h"
#include "FreeRTOS.h"
#include "crc.h"
#include "gpio.h"
#include "internal_flash.h"
#include "mc25lc512.h"
#include "printf.h"
#include "spi.h"
#include "task.h"
#include "usart.h"
#include <stdint.h>
#include <string.h>

#define LED_OFF 0x01
#define LED_RED 0x02
#define LED_GREEN 0x04
#define LED_BLUE 0x08

#define BOOTLOADER_CONFIG_ADDRESS 0x0000
#define FLAS_START_ADDRESS 0x08000000
#define FW_START_ADDRESS 0xC800
#define NOTIFICATION_TIMEOUT_MS 1000
#define BOOTLOADER_TIMEOUT_MS 10000

#define PACKET_SYNCH_SIZE 3
#define PACKET_SYNCH_DATA_1 'K'
#define PACKET_SYNCH_DATA_2 'D'
#define PACKET_SYNCH_DATA_3 'Y'
#define PACKET_INDEX_SIZE 1
#define PACKET_CRC_SIZE 4
#define PACKET_PAYLOAD_SIZE 1024
#define MAX_BUFFER_SIZE                                                        \
  (PACKET_PAYLOAD_SIZE + PACKET_SYNCH_SIZE + PACKET_INDEX_SIZE +               \
   PACKET_CRC_SIZE)

uint8_t glb_uart_buffer[MAX_BUFFER_SIZE * 2] = {0};
uint32_t global_uart_counter = 0;
uint8_t uart_it_byte = 0;
uint8_t ready_to_jump = 0; // 1 means jump is available
xeeprom *eeprom = NULL;
static TaskHandle_t xPacketProcessorTask = NULL;

typedef void (*pFunction)(void);

typedef enum {
  no_error = 0,
  waiting_new_packet,
  transfer_complete,
  error_occured
} xparse_status;

typedef struct {
  uint32_t fw_size;
  uint32_t fw_crc32;
} xbootloader_cfg;

typedef enum {
  LED_SET = GPIO_PIN_RESET,
  LED_RESET = GPIO_PIN_SET,
  LED_TOGGLE,
  LED_STATE_COUNT
} led_state;

void SystemClock_Config(void);
void uart_log(const char *format, ...);

static void jump_to_address(uint32_t address) {
  if (ready_to_jump != 1) {
    uart_log("no FW found to be jumped, wait here forever\n");
    while (1) {
    }
  }

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

static void led_control(uint8_t led_code, led_state led_state) {
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart3) {
    BaseType_t xHigherPriorityTaskWoken;

    HAL_UART_Receive_IT(&huart3, (uint8_t *)&uart_it_byte, 1);

    glb_uart_buffer[global_uart_counter++] = uart_it_byte;
    if (global_uart_counter >= MAX_BUFFER_SIZE) {
      if (global_uart_counter >= sizeof(glb_uart_buffer)) {
        global_uart_counter = 0;
      }
      // notify other task immediately
      xHigherPriorityTaskWoken = pdFALSE;

      xTaskNotifyFromISR(xPacketProcessorTask, global_uart_counter,
                         eSetValueWithOverwrite, &xHigherPriorityTaskWoken);

      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

static void packet_dump(uint8_t *buffer, uint32_t len) {
  if (!buffer) {
    return;
  }
  for (uint32_t i = 0; i < len; i++) {
    uart_log("data[%u]: %x\n", i, buffer[i]);
  }

  while (1) {
  }
}

static xparse_status
packet_parse_and_respond(uint8_t *buffer, uint32_t lenght,
                         uint8_t *number_of_written_pages) {
  uint32_t i = 0, crc = 0, starting_idx = 0;
  uint32_t crc_calculated = 0;
  uint8_t is_transfer_complete = 1;
  static uint16_t global_index = 0;

  // we are waiting fixed sized buffer all the time
  if (!buffer) {
    uart_log("args are wrong %d\n", __LINE__);
    return error_occured;
  }

  while (1) {
    if (starting_idx + 3 < lenght) {
      if (buffer[starting_idx + 0] == PACKET_SYNCH_DATA_1 &&
          buffer[starting_idx + 1] == PACKET_SYNCH_DATA_2 &&
          buffer[starting_idx + 2] == PACKET_SYNCH_DATA_3) {
        // we found
        break;
      }
    } else {
      // packet cannot be found
      uart_log("starting packet cannot be found\n");
      return waiting_new_packet;
    }
    starting_idx++;
  }

  /*
  packet format
  _________________________________________________________________________________________
  | 'KDY' (3 byte) | IDX (1 byte) | payload (1024 bytes) | CRC
  (4 bytes) |
  -----------------------------------------------------------------------------------------

  if the buffer is full of 0xCC, it means the FW flashed
  */

  for (i = 0; i < PACKET_PAYLOAD_SIZE; i++) {
    if (buffer[starting_idx + PACKET_SYNCH_SIZE + PACKET_INDEX_SIZE + i] !=
        0xCC) {
      is_transfer_complete = 0;
      break;
    }
  }

  if (is_transfer_complete) {
    uart_log("transfer complete\n");
    return transfer_complete;
  }

  if (buffer[starting_idx + PACKET_SYNCH_SIZE] != global_index) {
    uart_log("index wrong\n");
    packet_dump(&buffer[starting_idx], lenght);
    return error_occured;
  }

  crc_calculated = chksum_crc32_start();

  for (i = 0; i < PACKET_PAYLOAD_SIZE; i++) {
    uint8_t data =
        buffer[starting_idx + PACKET_SYNCH_SIZE + PACKET_INDEX_SIZE + i];
    crc_calculated = chksum_crc32_continue(crc_calculated, &data, sizeof(data));
  }

  crc_calculated = chksum_crc32_end(crc_calculated);
  memcpy(&crc,
         &buffer[starting_idx + PACKET_SYNCH_SIZE + PACKET_INDEX_SIZE +
                 PACKET_PAYLOAD_SIZE],
         PACKET_CRC_SIZE);

  if (crc != crc_calculated) {
    uart_log("crc wrong, crc: %x crc_calculated: %x\n", crc, crc_calculated);
    packet_dump(&buffer[starting_idx], lenght);
    return error_occured;
  }

  // now, packet is valid

  // save it to flash
  if (flash_write(FLAS_START_ADDRESS + FW_START_ADDRESS +
                      (global_index * FLASH_PAGE_SIZE),
                  (uint8_t *)&buffer[starting_idx + PACKET_SYNCH_SIZE +
                                     PACKET_INDEX_SIZE],
                  PACKET_PAYLOAD_SIZE)) {
    uart_log("flash_write() failed\n");
    return error_occured;
  }

  // now return to an answer to the FW flasher
  // format is like "OK:<idx>"

  uart_log("OK:%u\n", global_index);

  global_index++;

  *number_of_written_pages = global_index;

  taskENTER_CRITICAL();
  global_uart_counter = 0;
  taskEXIT_CRITICAL();

  return waiting_new_packet;
}

static uint8_t update_bootloader_config(uint8_t how_many_pages_written) {
  xbootloader_cfg bootloader_conf;
  uint32_t i = 0;
  uint32_t crc = chksum_crc32_start();

  for (i = 0; i < how_many_pages_written * FLASH_PAGE_SIZE; i++) {
    uint8_t data = 0;
    flash_read(FLAS_START_ADDRESS + FW_START_ADDRESS + i, (uint8_t *)&data,
               sizeof(data));
    crc = chksum_crc32_continue(crc, &data, sizeof(data));
  }

  crc = chksum_crc32_end(crc);

  bootloader_conf.fw_crc32 = crc;
  bootloader_conf.fw_size = how_many_pages_written * FLASH_PAGE_SIZE;

  uart_log("calculated crc: %x\n", bootloader_conf.fw_crc32);
  uart_log("FW size %x\n", bootloader_conf.fw_size);

  MC25LC512_Write(eeprom, BOOTLOADER_CONFIG_ADDRESS,
                  (uint8_t *)&bootloader_conf, sizeof(bootloader_conf));

  memset((uint8_t *)&bootloader_conf, 0, sizeof(bootloader_conf));

  MC25LC512_Read(eeprom, (uint16_t)BOOTLOADER_CONFIG_ADDRESS,
                 (uint8_t *)&bootloader_conf, sizeof(xbootloader_cfg));

  if (bootloader_conf.fw_crc32 != crc ||
      bootloader_conf.fw_size != (how_many_pages_written * FLASH_PAGE_SIZE)) {
    uart_log("eeprom cannot be updated\n");
    return 1; // error
  }

  return 0;
}

void packet_processor_task(void *data) {
  (void)data;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(NOTIFICATION_TIMEOUT_MS);
  BaseType_t xResult = pdFAIL;
  uint8_t number_of_written_pages = 0;
  uint32_t ulNotifiedValue;
  uint32_t timeout_counter = 0;
  xparse_status parse_status = no_error;

  while (1) {
    xResult =
        xTaskNotifyWait(pdFALSE,             /* Don't clear bits on entry. */
                        0, &ulNotifiedValue, /* Stores the notified value. */
                        xMaxBlockTime);
    if (xResult == pdPASS) {
      led_control(LED_RED, LED_TOGGLE);
      timeout_counter = 0; // we got a packet, clear counter

      // parse the packet
      parse_status = packet_parse_and_respond(glb_uart_buffer, ulNotifiedValue,
                                              &number_of_written_pages);
      if (parse_status == transfer_complete) {
        // update bootloader config
        uart_log("written page number %d\n", number_of_written_pages);
        if (update_bootloader_config(number_of_written_pages)) {
          uart_log("bootloader config update failed\n");
        } else {
          ready_to_jump = 1;
        }
      }

      taskYIELD();
    } else {
      if (parse_status == no_error) {
        uart_log("broadcasting\n");
      } else {
        uart_log("timeout\n");
      }
      led_control(LED_BLUE, LED_TOGGLE);
      timeout_counter++;

      if (timeout_counter >=
          (BOOTLOADER_TIMEOUT_MS / NOTIFICATION_TIMEOUT_MS)) {
        // it means BOOTLOADER_TIMEOUT_MS long slience period expired
        // in the mean time maybe we got the new FW or we had one already, does
        // not really matter jump to application if exists
        jump_to_address(FLAS_START_ADDRESS + FW_START_ADDRESS);
      }
    }
  }
}

static uint8_t check_fw_exists(xeeprom *eeprom,
                               xbootloader_cfg *bootloader_conf) {
  uint8_t ret = 0;

  if (!eeprom || !bootloader_conf) {
    uart_log("arg is null %d\n", __LINE__);
  }

  ret = MC25LC512_Read(eeprom, (uint16_t)BOOTLOADER_CONFIG_ADDRESS,
                       (uint8_t *)bootloader_conf, sizeof(xbootloader_cfg));
  if (ret) {
    uart_log("MC25LC512_Read() failed %d\n", __LINE__);
  }

  if (bootloader_conf->fw_size == 0 || bootloader_conf->fw_crc32 == 0) {
    return 0; // not exist
  }

  return 1;
}

static uint8_t check_fw_valid(xbootloader_cfg *bootloader_conf) {
  uint32_t i = 0;
  uint32_t crc = chksum_crc32_start();

  if (bootloader_conf->fw_size + FLAS_START_ADDRESS + FW_START_ADDRESS >
      FLASH_MAX_PAGE_BASE_ADDRESS) {
    return 0;
  }

  for (i = 0; i < bootloader_conf->fw_size; i++) {
    uint8_t data = 0;
    flash_read(FLAS_START_ADDRESS + FW_START_ADDRESS + i, (uint8_t *)&data,
               sizeof(data));
    crc = chksum_crc32_continue(crc, &data, sizeof(data));
  }

  crc = chksum_crc32_end(crc);

  uart_log("calculated CRC: %x\n", crc);
  uart_log("found CRC: %x\n", bootloader_conf->fw_crc32);
  uart_log("found fw size: %x\n", bootloader_conf->fw_size);

  if (bootloader_conf->fw_crc32 == crc) {
    return 1;
  }

  return 0; // not valid
}

int main(void) {
  xbootloader_cfg bootloader_conf;

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();

  HAL_UART_Receive_IT(&huart3, (uint8_t *)&uart_it_byte, 1);
  led_control(LED_GREEN | LED_RED | LED_BLUE, LED_RESET);

  uart_log("Bootloader Started!\n");

  eeprom = MC25LC512_Initilize(&hspi1, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin);
  if (eeprom == NULL) {
    uart_log("MC25LC512_Initilize() failed\n");
  }

  if (check_fw_exists(eeprom, &bootloader_conf)) {
    // lets verify the FW is valid
    if (check_fw_valid(&bootloader_conf)) {
      // there is a valid FW, so if no new FW come from uart,
      // then we are ready to jump
      ready_to_jump = 1;
      uart_log("Found a valid FW!\n");
    } else {
      uart_log("FW cannot be validated!\n");
    }
  } else {
    uart_log("FW is not exist!\n");
  }

  xTaskCreate(packet_processor_task, "PacketProcessorTask", 2048, NULL,
              tskIDLE_PRIORITY, &xPacketProcessorTask);

  vTaskStartScheduler();

  while (1) {
  }

  MC25LC512_Deinitilize(eeprom);
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
