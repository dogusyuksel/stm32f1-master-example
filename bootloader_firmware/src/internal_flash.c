#include "internal_flash.h"
#include "stm32f1xx_hal.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

extern void uart_log(const char *format, ...);

uint32_t page_erase(uint32_t starting_address) {
  FLASH_EraseInitTypeDef EraseInitStruct;

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = starting_address;
  EraseInitStruct.NbPages = 1;

  uint32_t pError = 0;
  return HAL_FLASHEx_Erase(&EraseInitStruct, &pError);
}

uint32_t clear_flash(uint32_t starting_address) {
  int i = 0, page_count = 0;

  /* Unlock the Flash Bank1 Program Erase controller */
  HAL_FLASH_Unlock();
  __disable_irq();

  /* Clear All pending flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

  /* Erase the FLASH pages */
  page_count =
      (FLASH_MAX_PAGE_BASE_ADDRESS - starting_address) / FLASH_PAGE_SIZE;
  for (i = 0; i < page_count; i++) {
    uint32_t pError = 0;
    if ((pError == page_erase(starting_address + (FLASH_PAGE_SIZE * i)))) {
      uart_log("err: %x pn: %d addr: %x line: %d\n", pError, i,
               starting_address + (FLASH_PAGE_SIZE * i), __LINE__);
      __enable_irq();
      HAL_FLASH_Lock();
      return pError;
    }
  }

  __enable_irq();
  HAL_FLASH_Lock();

  return 0;
}

void flash_read(uint32_t address, uint8_t *data, uint32_t len) {
  assert((data != NULL) && (len != 0));
  assert(((address + len) <= FLASH_MAX_PAGE_BASE_ADDRESS));

  uint32_t Address = address;
  uint32_t i = 0;
  for (i = 0; i < len; i++) {
    data[i] = (*(__IO uint8_t *)Address++);
  }
}

uint32_t flash_write(uint32_t address, uint8_t *data, uint32_t len) {
  uint32_t i = 0;

  // control the input
  assert((data != NULL) && (len != 0));
  assert((len == FLASH_PAGE_SIZE));
  assert((len % sizeof(uint32_t)) == 0);
  assert(((address + len) <= FLASH_MAX_PAGE_BASE_ADDRESS));
  assert((address % sizeof(uint32_t)) == 0);
  assert((address % FLASH_PAGE_SIZE) == 0);

  /* Unlock the Flash Bank1 Program Erase controller */
  HAL_FLASH_Unlock();
  __disable_irq();

  /* Clear All pending flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

  /* Erase the FLASH pages */
  uint32_t pError = 0;
  pError = page_erase(address);
  if (pError) {
    uart_log("err: %x addr: %x line: %d\n", pError, address, __LINE__);
    __enable_irq();
    HAL_FLASH_Lock();
    return pError;
  }

  /* Program Flash related whole page */
  uint32_t word = 0;
  for (i = 0; i < len; i += 4) {
    // memcpy(&word, &data[i], sizeof(uint32_t));
    word = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) |
           (data[i + 3] << 24);

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR |
                           FLASH_FLAG_WRPERR);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, word);
  }

  __enable_irq();
  HAL_FLASH_Lock();

  return 0;
}
