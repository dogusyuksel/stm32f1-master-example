#!/bin/bash

is_debug=1
copy_to_ram=1

usage() {
        echo "Usage: $0"
        echo "  [-r to specifcy Release build (default is Debug)]"
        echo "  [-f to specifcy copy to FLASH (default is RAM)]" } 1>&2; exit 1; }

while getopts ":rf" o; do
    case "${o}" in
        r)
            is_debug=0
            ;;
        f)
            copy_to_ram=0
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

echo "is_debug    = ${is_debug}"
echo "copy_to_ram = ${copy_to_ram}"

rm -rf build
rm -rf flash.sh
mkdir build
cd build
../../thirdparty/linting/format_check.sh ..

app_name="copy_to_ram_debug"
# if there is one arg, then consider it Release build
if [ "$is_debug" -eq 1 ]; then
    if [ "$copy_to_ram" -eq 1 ]; then
        cmake ..
    else
        app_name="copy_to_flash_debug"
        cmake -DCOPY_TO=FLASH ..
    fi
else
    if [ "$copy_to_ram" -eq 1 ]; then
        app_name="copy_to_ram_release"
        cmake -DCMAKE_BUILD_TYPE=Release ..
    else
        app_name="copy_to_flash_release"
        cmake -DCMAKE_BUILD_TYPE=Release -DCOPY_TO=FLASH ..
    fi
fi

make
arm-none-eabi-objcopy -O binary bootloader_copy_to_ram.elf $app_name.bin
cd -

if ! [ -f flash.sh ]; then
    echo "#!/bin/bash" >> flash.sh
    echo "st-flash --connect-under-reset write build/$app_name.bin 0x08000000" >> flash.sh
    chmod +x flash.sh
fi

if ! [ -f erase_flash.sh ]; then
    echo "#!/bin/bash" >> erase_flash.sh
    echo "st-flash erase 0x08000000 0x10000" >> erase_flash.sh
    chmod +x erase_flash.sh
fi