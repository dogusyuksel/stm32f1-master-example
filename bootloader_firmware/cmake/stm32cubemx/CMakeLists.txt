cmake_minimum_required(VERSION 3.22)

project(stm32cubemx)
add_library(stm32cubemx INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

target_compile_definitions(stm32cubemx INTERFACE 
	USE_HAL_DRIVER 
	STM32F103xB
    $<$<CONFIG:Debug>:DEBUG>
)

set(WORKSPACE_PATH "$ENV{WORKSPACE}")
set(THIRDPARTY_PATH "$ENV{THIRDPARTY}")

target_include_directories(stm32cubemx INTERFACE
    ../../inc
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Inc
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Inc/Legacy
    ${THIRDPARTY_PATH}/cmsis-header-stm32/stm32f1xx/Include
    ${THIRDPARTY_PATH}/CMSIS_5/CMSIS/Core/Include
    ${THIRDPARTY_PATH}/FreeRTOS-Kernel/include
    ${THIRDPARTY_PATH}/FreeRTOS-Kernel/portable/GCC/ARM_CM3
    ${THIRDPARTY_PATH}/custom_printf
)

file(GLOB free_rtos_all
     "${THIRDPARTY_PATH}/FreeRTOS-Kernel/*.c"
)

file(GLOB files_under_src
     "../../src/*.c"
)

target_sources(stm32cubemx INTERFACE
    ${free_rtos_all}
    ${files_under_src}
    ../../startup_stm32f103xb.s
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_gpio_ex.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_spi.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_rcc.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_rcc_ex.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_gpio.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_dma.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_cortex.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_pwr.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_flash.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_flash_ex.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_exti.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_tim.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_tim_ex.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_uart.c
    ${THIRDPARTY_PATH}/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c
    ${THIRDPARTY_PATH}/FreeRTOS-Kernel/portable/MemMang/heap_3.c
    ${THIRDPARTY_PATH}/custom_printf/printf.c
)

target_link_directories(stm32cubemx INTERFACE
)

target_link_libraries(stm32cubemx INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


