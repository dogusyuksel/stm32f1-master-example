#!/bin/bash

# with any argument, it will build for libcsp

if [ "$#" -eq 0 ]; then
    if [ -d "generated_files" ]; then
        rm -rf generated_files
    fi
    /workspace/thirdparty/libcanard/dsdl_compiler/libcanard_dsdlc ./nodes --incdir /workspace/thirdparty/libcanard/dsdl_compiler/pyuavcan/uavcan --outdir ./generated_files
    if [[ $? = 0 ]]; then
        echo "success"
    else
        # then try with my docker
        cd ..
        /workspace/thirdparty/docker/run_docker.sh golden-sample:latest "cd /workspace && /workspace/thirdparty/libcanard/dsdl_compiler/libcanard_dsdlc ./application_firmware/nodes --incdir /workspace/thirdparty/libcanard/dsdl_compiler/pyuavcan/uavcan --outdir ./application_firmware/generated_files"
        cd -
    fi
else
    rm -rf generated_files
    cd /workspace/thirdparty/libcsp
    rm -rf build
    ./waf configure --with-os=freertos --toolchain=arm-none-eabi- --includes=/workspace/thirdparty/FreeRTOS-Kernel,/workspace/thirdparty/FreeRTOS-Kernel/include,/workspace/thirdparty/FreeRTOS-Kernel/portable/GCC/ARM_CM3,/workspace/application_firmware/inc/libcsp_freertos
    ./waf clean
    ./waf build
    cd -
fi

rm -rf build
rm -rf flash.sh
mkdir build
cd build

# if there is one arg, then consider it will be build for libcsp
if [ "$#" -eq 1 ]; then
    cmake -DLIBCSP=libcsp ..
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


if ! [ -f erase_flash.sh ]; then
    echo "#!/bin/bash" >> erase_flash.sh
    echo "st-flash erase 0x08000000 0x10000" >> erase_flash.sh
    chmod +x erase_flash.sh
fi

