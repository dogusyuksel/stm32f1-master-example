#!/bin/bash

# important!! first call must have '-b' option
./docker_ctl.sh -b -c 'cd /workspace/bootloader_firmware && ./build.sh'
./docker_ctl.sh -c 'cd /workspace/bootloader_firmware && ceedling clean && ceedling test:all'
./docker_ctl.sh -c 'cd /workspace/bootloader_tool && make'
./docker_ctl.sh -c 'cd /workspace/application_firmware && ./build.sh'
./docker_ctl.sh -c 'cd /workspace/application_tool && ./build.sh'
./docker_ctl.sh -c 'cd /workspace/bootloader_copy_to_ram && ./build.sh'
./docker_ctl.sh -c 'cd /workspace/application_boot_from_ram && ./build.sh'
