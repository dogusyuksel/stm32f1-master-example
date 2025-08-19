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

target_include_directories(stm32cubemx INTERFACE
    ../../inc
    /workspace/thirdparty/libcanard
    /workspace/thirdparty/libcanard/drivers/stm32
    ../../generated_files
    /workspace/thirdparty/STM32F103X_HAL/Inc
    /workspace/thirdparty/STM32F103X_HAL/Inc/Legacy
    /workspace/thirdparty/cmsis-header-stm32/stm32f1xx/Include
    /workspace/thirdparty/CMSIS_5/CMSIS/Core/Include
    /workspace/thirdparty/FreeRTOS-Kernel/include
    /workspace/thirdparty/FreeRTOS-Kernel/portable/GCC/ARM_CM3
    /workspace/thirdparty/custom_printf
)

file(GLOB free_rtos_all
     "/workspace/thirdparty/FreeRTOS-Kernel/*.c"
)

file(GLOB files_under_src
     "../../src/*.c"
)

file(GLOB_RECURSE generated_files
     "../../generated_files/*.c"
)

target_sources(stm32cubemx INTERFACE
    ${free_rtos_all}
    ${files_under_src}
    ${generated_files}
    ../../startup_stm32f103xb.s
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_gpio_ex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_adc.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_adc_ex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_rcc.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_rcc_ex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_gpio.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_dma.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_cortex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_pwr.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_flash.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_flash_ex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_exti.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_can.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_i2c.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_rtc.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_rtc_ex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_tim.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_tim_ex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_uart.c
    /workspace/thirdparty/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c
    /workspace/thirdparty/FreeRTOS-Kernel/portable/MemMang/heap_3.c
    /workspace/thirdparty/custom_printf/printf.c
    /workspace/thirdparty/libcanard/canard.c
    /workspace/thirdparty/libcanard/drivers/stm32/canard_stm32.c
)

target_link_directories(stm32cubemx INTERFACE
)

target_link_libraries(stm32cubemx INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


