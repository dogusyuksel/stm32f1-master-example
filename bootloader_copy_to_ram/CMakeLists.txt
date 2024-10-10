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
if(NOT COPY_TO)
    set(COPY_TO "RAM")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME bootloader_copy_to_ram)

# Include toolchain file
include("gcc-arm-none-eabi.cmake")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"../STM32F103C8Tx_FLASH.ld\"")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

file(GLOB files_under_src
     "src/*.c"
)

add_executable(${CMAKE_PROJECT_NAME}
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_gpio_ex.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_tim.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_tim_ex.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_uart.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_rcc.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_rcc_ex.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_gpio.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_dma.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_cortex.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_pwr.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_flash.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_flash_ex.c
    ../thirdparty/STM32F103X_HAL/Src/stm32f1xx_hal_exti.c
    ${files_under_src}
    startup_stm32f103xb.s
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC SYSTEM
    inc
    ../thirdparty/STM32F103X_HAL/Inc
    ../thirdparty/STM32F103X_HAL/Inc/Legacy
    ../thirdparty/cmsis-header-stm32/stm32f1xx/Include
    ../thirdparty/CMSIS_5/CMSIS/Core/Include
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
