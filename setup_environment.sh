#!/bin/bash

update_repo () {
   cd $1
   git clean -fd
   git checkout *
   git pull
   cd -
}

if [ ! -d "thirdparty" ]; then
    mkdir thirdparty
fi

if [ ! -d "thirdparty/STM32F103X_HAL" ];
then
    git clone https://github.com/STMicroelectronics/stm32f1xx_hal_driver.git thirdparty/STM32F103X_HAL
else
    update_repo "thirdparty/STM32F103X_HAL"
fi

if [ ! -d "thirdparty/cmsis-header-stm32" ];
then
    git clone https://github.com/modm-io/cmsis-header-stm32.git thirdparty/cmsis-header-stm32
else
    update_repo "thirdparty/cmsis-header-stm32"
fi

if [ ! -d "thirdparty/CMSIS_5" ]; then
    git clone https://github.com/ARM-software/CMSIS_5.git thirdparty/CMSIS_5
else
    update_repo "thirdparty/CMSIS_5"
fi

if [ ! -d "thirdparty/FreeRTOS-Kernel" ]; then
    git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git thirdparty/FreeRTOS-Kernel
else
    update_repo "thirdparty/FreeRTOS-Kernel"
fi

if [ ! -d "thirdparty/linting" ]; then
    git clone https://github.com/dogusyuksel/embedded_linting.git thirdparty/linting
else
    update_repo "thirdparty/linting"
fi

if [ ! -d "thirdparty/docker" ]; then
    git clone https://github.com/dogusyuksel/embedded_docker.git thirdparty/docker
else
    update_repo "thirdparty/docker"
fi

if [ ! -d "thirdparty/custom_printf" ]; then
    git clone https://github.com/mpaland/printf.git thirdparty/custom_printf
else
    update_repo "thirdparty/custom_printf"
fi

if [ ! -d "thirdparty/openocd" ]; then
    git clone https://github.com/STMicroelectronics/OpenOCD.git thirdparty/openocd
    cat ./thirdparty/openocd/tcl/board/stm32f103c8_blue_pill.cfg | sed -e "s/set FLASH_SIZE 0x20000/set FLASH_SIZE 0x10000/" > ./thirdparty/openocd/tcl/board/stm32f103c8_custom.cfg
fi

if [ ! -d "thirdparty/libcanard" ]; then
    git clone https://github.com/OpenCyphal/libcanard.git thirdparty/libcanard
    cd thirdparty/libcanard
    git checkout 43de1c4966b8d1e5d57978949d63e697f045b358
    git submodule update --init --recursive
    cd -
    patch -d . -p1 < ./patches/transport.py.patch
    patch -d . -p1 < ./patches/canard_stm32.patch
fi

exit 0