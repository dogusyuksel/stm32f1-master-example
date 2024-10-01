#!/bin/bash

if [ -d "generated_files" ]; then
    sudo rm -rf generated_files
fi
./../thirdparty/libcanard/dsdl_compiler/libcanard_dsdlc ./nodes --incdir ./../thirdparty/libcanard/dsdl_compiler/pyuavcan/uavcan --outdir ./generated_files
if [[ $? = 0 ]]; then
    echo "success"
else
    # then try with my docker
    cd ..
    ./thirdparty/docker/run_docker.sh golden-sample:latest "cd /workspace && ./thirdparty/libcanard/dsdl_compiler/libcanard_dsdlc ./application_firmware/nodes --incdir ./thirdparty/libcanard/dsdl_compiler/pyuavcan/uavcan --outdir ./application_firmware/generated_files"
    cd -
fi

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
arm-none-eabi-objcopy -O binary application_firmware.elf application_firmware.bin
cd -

if ! [ -f flash.sh ]; then
    echo "st-flash --connect-under-reset write build/application_firmware.bin 0x08000000" >> flash.sh
    chmod +x flash.sh
fi

