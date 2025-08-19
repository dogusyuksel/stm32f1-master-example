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
rm -rf flash.sh
mkdir build
cd build
/workspace/thirdparty/linting/format_check.sh ..

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

if ! [ -f erase_flash.sh ]; then
    echo "#!/bin/bash" >> erase_flash.sh
    echo "st-flash erase 0x08000000 0x10000" >> erase_flash.sh
    chmod +x erase_flash.sh
fi

