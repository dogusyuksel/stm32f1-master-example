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
    ../../Core/Inc
    /workspace/thirdparty/STM32F103X_HAL/Inc
    /workspace/thirdparty/STM32F103X_HAL/Inc/Legacy
    /workspace/thirdparty/CMSIS_5/CMSIS/Core/Include
    /workspace/thirdparty/cmsis-header-stm32/stm32f1xx/Include
)

target_sources(stm32cubemx INTERFACE
    ../../Core/Src/main.c
    ../../Core/Src/gpio.c
    ../../Core/Src/i2c.c
    ../../Core/Src/spi.c
    ../../Core/Src/usart.c
    ../../Core/Src/stm32f1xx_it.c
    ../../Core/Src/stm32f1xx_hal_msp.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_gpio_ex.c
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
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_i2c.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_spi.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_tim.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_tim_ex.c
    /workspace/thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_uart.c
    ../../Core/Src/system_stm32f1xx.c
    ../../Core/Src/sysmem.c
    ../../Core/Src/syscalls.c
    ../../startup_stm32f103xb.s
)

target_link_directories(stm32cubemx INTERFACE
)

target_link_libraries(stm32cubemx INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


