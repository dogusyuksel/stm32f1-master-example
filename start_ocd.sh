#!/bin/bash

st-info --probe

openocd -f /thirdparty/openocd/tcl/interface/stlink.cfg -f /thirdparty/openocd/tcl/board/stm32f103c8_custom.cfg

# then in another terminal, run below
# gdb-multiarch <your_code>.elf
# target extended-remote localhost:3333

exit 0
