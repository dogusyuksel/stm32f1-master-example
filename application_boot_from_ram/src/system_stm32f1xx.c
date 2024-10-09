

#include "stm32f1xx.h"

#if !defined(HSE_VALUE)
#define HSE_VALUE                                                              \
  8000000U /*!< Default value of the External oscillator in Hz.                \
                This value can be provided and adapted by the user             \
              application. */
#endif     /* HSE_VALUE */

#if !defined(HSI_VALUE)
#define HSI_VALUE                                                              \
  8000000U /*!< Default value of the Internal oscillator in Hz.                \
                This value can be provided and adapted by the user             \
              application. */
#endif     /* HSI_VALUE */

#if defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F101xG) ||    \
    defined(STM32F103xE) || defined(STM32F103xG)
#endif /* STM32F100xE || STM32F101xE || STM32F101xG || STM32F103xE ||          \
          STM32F103xG */

#if defined(USER_VECT_TAB_ADDRESS)
#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS                                                  \
  SRAM_BASE /*!< Vector Table base address field.                              \
                 This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET                                                        \
  0x1800U /*!< Vector Table base offset field.                                 \
                   This value must be a multiple of 0x200. */
#else
#define VECT_TAB_BASE_ADDRESS                                                  \
  FLASH_BASE /*!< Vector Table base address field.                             \
                  This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET                                                        \
  0x0000U /*!< Vector Table base offset field.                                 \
                   This value must be a multiple of 0x200. */
#endif    /* VECT_TAB_SRAM */
#endif    /* USER_VECT_TAB_ADDRESS */

uint32_t SystemCoreClock = 16000000;
const uint8_t AHBPrescTable[16U] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8U] = {0, 0, 0, 0, 1, 2, 3, 4};

#if defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F101xG) ||    \
    defined(STM32F103xE) || defined(STM32F103xG)
#ifdef DATA_IN_ExtSRAM
static void SystemInit_ExtMemCtl(void);
#endif /* DATA_IN_ExtSRAM */
#endif /* STM32F100xE || STM32F101xE || STM32F101xG || STM32F103xE ||          \
          STM32F103xG */

void SystemInit(void) {
#if defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F101xG) ||    \
    defined(STM32F103xE) || defined(STM32F103xG)
#ifdef DATA_IN_ExtSRAM
  SystemInit_ExtMemCtl();
#endif /* DATA_IN_ExtSRAM */
#endif

#if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = VECT_TAB_BASE_ADDRESS |
              VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#endif                         /* USER_VECT_TAB_ADDRESS */
}

void SystemCoreClockUpdate(void) {
  uint32_t tmp = 0U, pllmull = 0U, pllsource = 0U;

#if defined(STM32F105xC) || defined(STM32F107xC)
  uint32_t prediv1source = 0U, prediv1factor = 0U, prediv2factor = 0U,
           pll2mull = 0U;
#endif /* STM32F105xC */

#if defined(STM32F100xB) || defined(STM32F100xE)
  uint32_t prediv1factor = 0U;
#endif /* STM32F100xB or STM32F100xE */

  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch (tmp) {
  case 0x00U: /* HSI used as system clock */
    SystemCoreClock = HSI_VALUE;
    break;
  case 0x04U: /* HSE used as system clock */
    SystemCoreClock = HSE_VALUE;
    break;
  case 0x08U: /* PLL used as system clock */

    pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
    pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;

#if !defined(STM32F105xC) && !defined(STM32F107xC)
    pllmull = (pllmull >> 18U) + 2U;

    if (pllsource == 0x00U) {
      SystemCoreClock = (HSI_VALUE >> 1U) * pllmull;
    } else {
#if defined(STM32F100xB) || defined(STM32F100xE)
      prediv1factor = (RCC->CFGR2 & RCC_CFGR2_PREDIV1) + 1U;
      SystemCoreClock = (HSE_VALUE / prediv1factor) * pllmull;
#else
      if ((RCC->CFGR & RCC_CFGR_PLLXTPRE) !=
          (uint32_t)RESET) { /* HSE oscillator clock divided by 2 */
        SystemCoreClock = (HSE_VALUE >> 1U) * pllmull;
      } else {
        SystemCoreClock = HSE_VALUE * pllmull;
      }
#endif
    }
#else
    pllmull = pllmull >> 18U;

    if (pllmull != 0x0DU) {
      pllmull += 2U;
    } else { /* PLL multiplication factor = PLL input clock * 6.5 */
      pllmull = 13U / 2U;
    }

    if (pllsource == 0x00U) {
      SystemCoreClock = (HSI_VALUE >> 1U) * pllmull;
    } else { /* PREDIV1 selected as PLL clock entry */

      prediv1source = RCC->CFGR2 & RCC_CFGR2_PREDIV1SRC;
      prediv1factor = (RCC->CFGR2 & RCC_CFGR2_PREDIV1) + 1U;

      if (prediv1source == 0U) {
        SystemCoreClock = (HSE_VALUE / prediv1factor) * pllmull;
      } else { /* PLL2 clock selected as PREDIV1 clock entry */

        prediv2factor = ((RCC->CFGR2 & RCC_CFGR2_PREDIV2) >> 4U) + 1U;
        pll2mull = ((RCC->CFGR2 & RCC_CFGR2_PLL2MUL) >> 8U) + 2U;
        SystemCoreClock =
            (((HSE_VALUE / prediv2factor) * pll2mull) / prediv1factor) *
            pllmull;
      }
    }
#endif /* STM32F105xC */
    break;

  default:
    SystemCoreClock = HSI_VALUE;
    break;
  }

  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4U)];
  SystemCoreClock >>= tmp;
}

#if defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F101xG) ||    \
    defined(STM32F103xE) || defined(STM32F103xG)
#ifdef DATA_IN_ExtSRAM
void SystemInit_ExtMemCtl(void) {
  __IO uint32_t tmpreg;

  RCC->AHBENR = 0x00000114U;

  tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_FSMCEN);

  RCC->APB2ENR = 0x000001E0U;

  tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPDEN);

  (void)(tmpreg);

  GPIOD->CRL = 0x44BB44BBU;
  GPIOD->CRH = 0xBBBBBBBBU;

  GPIOE->CRL = 0xB44444BBU;
  GPIOE->CRH = 0xBBBBBBBBU;

  GPIOF->CRL = 0x44BBBBBBU;
  GPIOF->CRH = 0xBBBB4444U;

  GPIOG->CRL = 0x44BBBBBBU;
  GPIOG->CRH = 0x444B4B44U;

  FSMC_Bank1->BTCR[4U] = 0x00001091U;
  FSMC_Bank1->BTCR[5U] = 0x00110212U;
}
#endif /* DATA_IN_ExtSRAM */
#endif /* STM32F100xE || STM32F101xE || STM32F101xG || STM32F103xE ||          \
          STM32F103xG */
