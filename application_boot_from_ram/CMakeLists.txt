cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()


# Define the boot type
if(NOT BOOT_FROM)
    set(BOOT_FROM "RAM")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME application_boot_from_ram)

# Include toolchain file
include("gcc-arm-none-eabi.cmake")

if(BOOT_FROM MATCHES RAM)
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"../STM32F103C8Tx_RAM.ld\"")
else()
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"../STM32F103C8Tx_FLASH.ld\"")
endif()

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

set(WORKSPACE_PATH "$ENV{WORKSPACE}")
set(THIRDPARTY_PATH "$ENV{THIRDPARTY}")

add_executable(${CMAKE_PROJECT_NAME}
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_gpio_ex.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_tim.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_tim_ex.c
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Src/stm32f1xx_hal_uart.c
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
    src/gpio.c
    src/main.c
    src/stm32f1xx_hal_msp.c
    src/stm32f1xx_hal_timebase_tim.c
    src/stm32f1xx_it.c
    src/syscalls.c
    src/sysmem.c
    src/usart.c
    src/system_stm32f1xx.c
    startup_stm32f103xb.s
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC SYSTEM
    inc
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Inc
    ${THIRDPARTY_PATH}/STM32F103X_HAL/Inc/Legacy
    ${THIRDPARTY_PATH}/cmsis-header-stm32/stm32f1xx/Include
    ${THIRDPARTY_PATH}/CMSIS_5/CMSIS/Core/Include
)



target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC SYSTEM
	USE_HAL_DRIVER 
	STM32F103xB
    $<$<CONFIG:Debug>:DEBUG>
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()
