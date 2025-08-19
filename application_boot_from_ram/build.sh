#!/bin/bash

is_debug=1
is_boot_from_ram=1

usage() {
        echo "Usage: $0"
        echo "  [-r to specifcy Release build (default is Debug)]"
        echo "  [-f to specifcy run from FLASH (default is RAM)]" } 1>&2; exit 1; }

while getopts ":rf" o; do
    case "${o}" in
        r)
            is_debug=0
            ;;
        f)
            is_boot_from_ram=0
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

echo "is_debug         = ${is_debug}"
echo "is_boot_from_ram = ${is_boot_from_ram}"

rm -rf build
rm -rf flash.sh
mkdir build
cd build
/workspace/thirdparty/linting/format_check.sh ..

app_name="boot_from_ram_debug"
# if there is one arg, then consider it Release build
if [ "$is_debug" -eq 1 ]; then
    if [ "$is_boot_from_ram" -eq 1 ]; then
        cmake ..
    else
        app_name="boot_from_flash_debug"
        cmake -DBOOT_FROM=FLASH ..
    fi
else
    if [ "$is_boot_from_ram" -eq 1 ]; then
        app_name="boot_from_ram_release"
        cmake -DCMAKE_BUILD_TYPE=Release ..
    else
        app_name="boot_from_flash_release"
        cmake -DCMAKE_BUILD_TYPE=Release -DBOOT_FROM=FLASH ..
    fi
fi

make
arm-none-eabi-objcopy -O binary application_boot_from_ram.elf $app_name.bin
cd -

if ! [ -f flash.sh ]; then
    echo "#!/bin/bash" >> flash.sh
    echo "if [ \"\$#\" -eq 1 ]; then" >> flash.sh
    echo "    st-flash --connect-under-reset write build/$app_name.bin \$1" >> flash.sh
    echo "else" >> flash.sh
    echo "    st-flash --connect-under-reset write build/$app_name.bin 0x0800C800" >> flash.sh
    echo "fi" >> flash.sh
    chmod +x flash.sh
fi

if ! [ -f erase_flash.sh ]; then
    echo "#!/bin/bash" >> erase_flash.sh
    echo "st-flash erase 0x08000000 0x10000" >> erase_flash.sh
    chmod +x erase_flash.sh
fi



