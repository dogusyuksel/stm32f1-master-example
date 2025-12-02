#!/bin/bash
set -euo pipefail

# important!! first call must have '-b' option
./docker_ctl.sh -b -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace && ./setup_env.sh'
./docker_ctl.sh -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace/bootloader_firmware && ./build.sh'
./docker_ctl.sh -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace/bootloader_firmware && ceedling clean && ceedling test:all'
./docker_ctl.sh -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace/bootloader_tool && make'
./docker_ctl.sh -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace/application_firmware && ./build.sh'
./docker_ctl.sh -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace/application_tool && ./build.sh'
./docker_ctl.sh -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace/bootloader_copy_to_ram && ./build.sh'
./docker_ctl.sh -c 'export WORKSPACE="/workspace" && export THIRDPARTY="/workspace/thirdparty" && cd /workspace/application_boot_from_ram && ./build.sh'
