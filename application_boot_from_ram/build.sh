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
arm-none-eabi-objcopy -O binary application_boot_from_ram.elf application_boot_from_ram.bin
cd -

if ! [ -f flash.sh ]; then
    echo "#!/bin/bash" >> flash.sh >> flash.sh
    echo "if [ \"\$#\" -eq 1 ]; then" >> flash.sh
    echo "    st-flash --connect-under-reset write build/application_boot_from_ram.bin \$1" >> flash.sh
    echo "else" >> flash.sh
    echo "    st-flash --connect-under-reset write build/application_boot_from_ram.bin 0x0800C800" >> flash.sh
    echo "fi" >> flash.sh
    chmod +x flash.sh
fi

