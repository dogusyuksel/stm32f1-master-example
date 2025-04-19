#ifdef TEST

#include "unity.h"
#include "mc25lc512.h"

#include "mock_stm32f1xx_hal.h"
#include "mock_stm32f1xx_hal_spi.h"
#include "mock_stm32f1xx_hal_gpio.h"



void setUp(void)
{
}

void tearDown(void)
{
}

void test_MC25LC512_Deinitilize(void) {
    xeeprom *eeprom = NULL;

    MC25LC512_Deinitilize(eeprom);
}

#endif // TEST
