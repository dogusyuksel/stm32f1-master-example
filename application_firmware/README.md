# STM32F103 UART BOOTLOADER with HAL and RTOS

This project is a container project to work on the following contents;

1. github actions
2. I2C usage with lsm6dsm
3. CANBUS usage
4. UART usage
5. RTOS usage and multitasking
6. RTC Usage
7. IO operations on STM
8. HAL Driver usage
9. PPS Generation
10. ADC Usage

## How to Build

More detailed steps can be found in the GHA yml file but basically;

1. You need to setup the environment

```
cd ..
./setup_environment.sh
```

2. To build firmware

```
./build.sh
or
./build Release
```

3. To build CAN PC Sniffer

```
cd ../application_tool
./build.sh
```

## How to Use

To execute the can sniffer, you need to give interface name. Check application's "help" menu


## How to Debug

1. You can execute the "start_ocd.sh" to start the OpenOCD.
2. Proper environent setting for OpenOCD, you may follow thirdparty/docker/Dockerfile
3. Then follow the instructions inside rhe "start_ocd.sh" script
4. For more gdb usage notes, please check "docs"

## Compile as an Application FW

1. Check "application_firmware/STM32F103C8Tx_FLASH.ld" file and fix line something else (0x800C800 for eg) and decrease the lenght relatively (14K in our case)

```
FLASH (rx)      : ORIGIN = 0x800C800, LENGTH = 64K
```

2. goto application_firmware/src/system_stm32f1xx.c and fix "#define VECT_TAB_OFFSET" variables with the same value above

```
#define VECT_TAB_OFFSET 0xC800U (for eg)
```

Because of our bootloader + application sizes does not fit for 64KB Flash, we cannot do that now but that is OK. Ultimate goal was not that.
