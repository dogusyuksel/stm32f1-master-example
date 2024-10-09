#!/bin/bash

is_debug=1

usage() {
        echo "Usage: $0"
        echo "  [-r to specifcy Release build (default is Debug)]" } 1>&2; exit 1; }

while getopts ":r" o; do
    case "${o}" in
        r)
            is_debug=0
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

echo "is_debug   = ${is_debug}"


rm -rf build
mkdir build
cd build
../../thirdparty/linting/format_check.sh ..

# if there is one arg, then consider it Release build
if [ "$is_debug" -eq 1 ]; then
    cmake ..
else
    cmake -DCMAKE_BUILD_TYPE=Release ..
fi

make
arm-none-eabi-objcopy -O binary bootloader_firmware.elf bootloader_firmware.bin
cd -

if ! [ -f flash.sh ]; then
    echo "st-flash --connect-under-reset write build/bootloader_firmware.bin 0x08000000" >> flash.sh
    chmod +x flash.sh
fi

