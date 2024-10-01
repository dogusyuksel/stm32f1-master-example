#!/bin/bash

rm -rf build
mkdir build
cd build
../../thirdparty/linting/format_check.sh ..

# if there is one arg, then consider it Release build
if [ "$#" -eq 1 ]; then
    cmake -DCMAKE_BUILD_TYPE=Release ..
else
    cmake ..
fi

make
arm-none-eabi-objcopy -O binary bootloader_firmware.elf bootloader_firmware.bin
cd -

if ! [ -f flash.sh ]; then
    echo "st-flash --connect-under-reset write build/bootloader_firmware.bin 0x08000000" >> flash.sh
    chmod +x flash.sh
fi

