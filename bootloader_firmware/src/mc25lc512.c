#include "mc25lc512.h"

static void MC25LC512_CS(xeeprom *eeprom, unsigned char CS_Status) {

  if (eeprom == NULL) {
    return;
  }

  // For Cs of the EEprom
  if (CS_Status == EEPROM_CS_PIN_RESET) {
    // Zero reset the Chip
    HAL_GPIO_WritePin(eeprom->cs_port, eeprom->cs_pin, GPIO_PIN_RESET);
    HAL_Delay(5);
  } else { // One Set the chip
    HAL_GPIO_WritePin(eeprom->cs_port, eeprom->cs_pin, GPIO_PIN_SET);
    HAL_Delay(5);
  }
}

static void MC25LC512_WriteEnableOrDisable(xeeprom *eeprom,
                                           unsigned char EnableOrDisable) {
  unsigned char SendOneByte = 0;

  if (eeprom == NULL) {
    return;
  }

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_RESET); // Reset The spi Chip //Reset means Enable

  if (EnableOrDisable == EEPROM_Enable) {
    SendOneByte = MC25LCxxx_SPI_WREN;
  } else {
    SendOneByte = MC25LCxxx_SPI_WRDI;
  }
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200);
  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_SET); // Set The spi Chip //Set means Disable
}

static unsigned char MC25LC512_ReleaseDeepPowerDownMode(xeeprom *eeprom) {

  unsigned char SendOneByte;
  uint8_t RecieveByteOfReleaseDeepPowerMode = 0;
  SendOneByte = MC25LCxxx_SPI_RDID;

  if (eeprom == NULL) {
    return 0;
  }

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_RESET); // Reset The spi Chip //Reset means Enable

  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200);

  HAL_SPI_Receive(eeprom->hspi, &RecieveByteOfReleaseDeepPowerMode, 1,
                  200); // Address of Manufaturer id High
  HAL_SPI_Receive(eeprom->hspi, &RecieveByteOfReleaseDeepPowerMode, 1,
                  200); // Address of Manufaturer id Low
  HAL_SPI_Receive(eeprom->hspi, &RecieveByteOfReleaseDeepPowerMode, 1,
                  200); // Manufaturer id
  eeprom->manufacturer_id = RecieveByteOfReleaseDeepPowerMode;

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_SET); // Set The spi Chip //Set means Disable

  return RecieveByteOfReleaseDeepPowerMode;
}

static unsigned char MC25LC512_ReadStatusRegister(xeeprom *eeprom) {

  unsigned char SendOneByte = 0;
  unsigned char ReceiveOneByte;
  SendOneByte = MC25LCxxx_SPI_RDSR;

  if (eeprom == NULL) {
    return 0;
  }

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_RESET); // Reset The spi Chip //Reset means Enable
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200);
  HAL_SPI_Receive(eeprom->hspi, &ReceiveOneByte, 1,
                  200); // Address of Manufaturer id High
  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_SET); // Set The spi Chip //Set means Disable
  return ReceiveOneByte;
}

void MC25LC512_Deinitilize(xeeprom *eeprom) {
  if (eeprom) {
    free(eeprom);
  }
}

xeeprom *MC25LC512_Initilize(SPI_HandleTypeDef *hspi_, GPIO_TypeDef *GPIOx_,
                             uint16_t GPIO_Pin_) {
  if (hspi_ == NULL || GPIOx_ == NULL) {
    return NULL;
  }

  xeeprom *eeprom = calloc(1, sizeof(xeeprom));
  if (eeprom == NULL) {
    return NULL;
  }

  eeprom->hspi = hspi_;
  eeprom->cs_port = GPIOx_;
  eeprom->cs_pin = GPIO_Pin_;

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_SET); // Reset The spi Chip //Reset means Enable
  MC25LC512_ReleaseDeepPowerDownMode(eeprom);
  MC25LC512_ReadStatusRegister(eeprom);
  MC25LC512_WriteEnableOrDisable(eeprom, EEPROM_Enable);

  if (eeprom->manufacturer_id != MANUFACTURER_ID) {
    return NULL;
  }

  HAL_Delay(150);

  return eeprom;
}

void MC25LC512_Write(xeeprom *eeprom, uint16_t AddresOfData,
                     unsigned char *WriteArrayOfEEProm,
                     unsigned short SizeOfArray) {

  unsigned char SendOneByte;

  if (eeprom == NULL) {
    return;
  }

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_RESET); // Reset The spi Chip //Reset means Enable
  HAL_Delay(1);
  SendOneByte = MC25LCxxx_SPI_WRITE;
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200);
  SendOneByte = AddresOfData >> 8;
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200); // High byte of address
  SendOneByte = AddresOfData & 0x00FF;
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200); // Low byte of address

  HAL_SPI_Transmit(eeprom->hspi, WriteArrayOfEEProm, SizeOfArray,
                   SizeOfArray * 50);
  HAL_Delay(4);
  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_SET); // Reset The spi Chip //Reset means Enable
  MC25LC512_WriteEnableOrDisable(eeprom, EEPROM_Enable);
}

unsigned char MC25LC512_Read(xeeprom *eeprom, uint16_t AddresOfData,
                             unsigned char *DataArrayOfEEProm,
                             unsigned short SizeOfArray) {

  unsigned char SendOneByte;

  if (eeprom == NULL) {
    return 0;
  }

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_RESET); // Reset The spi Chip //Reset means Enable
  HAL_Delay(1);
  SendOneByte = MC25LCxxx_SPI_READ; // Config the Device
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200);

  SendOneByte = AddresOfData >> 8;
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200); // High byte of address
  SendOneByte = AddresOfData & 0x00FF;
  HAL_SPI_Transmit(eeprom->hspi, &SendOneByte, 1, 200); // Low byte of address

  HAL_SPI_Receive(eeprom->hspi, DataArrayOfEEProm, SizeOfArray,
                  SizeOfArray * 30); // Receive Amount of  Data from EEPROM

  MC25LC512_CS(eeprom,
               EEPROM_CS_PIN_SET); // Reset The spi Chip //Reset means Enable
  HAL_Delay(1);
  return 0;
}
