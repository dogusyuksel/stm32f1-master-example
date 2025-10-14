#!/bin/bash

git config --global --add safe.directory /workspace/thirdparty/CMSIS_5
git config --global --add safe.directory /workspace/thirdparty/FreeRTOS-Kernel
git config --global --add safe.directory /workspace/thirdparty/STM32F103X_HAL
git config --global --add safe.directory /workspace/thirdparty/cmsis-header-stm32
git config --global --add safe.directory /workspace/thirdparty/custom_printf
git config --global --add safe.directory /workspace/thirdparty/libcanard
git config --global --add safe.directory /workspace/thirdparty/linting
git config --global --add safe.directory /workspace/thirdparty/openocd
git config --global --add safe.directory /workspace/thirdparty/libcsp

cd /workspace && git submodule update --init --recursive
# cd /workspace/thirdparty/libcanard && git checkout 43de1c4966b8d1e5d57978949d63e697f045b358
cd /workspace/thirdparty/libcanard && git submodule update --init --recursive
cd /workspace/thirdparty && patch -d . -p1 < canard_stm32.patch && patch -d . -p1 < transport.py.patch
cd /workspace && cat thirdparty/openocd/tcl/board/stm32f103c8_blue_pill.cfg | sed -e "s/set FLASH_SIZE 0x20000/set FLASH_SIZE 0x10000/" > thirdparty/openocd/tcl/board/stm32f103c8_custom.cfg
cd /workspace
