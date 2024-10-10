# STM32F103 Boot From RAM Example with HAL Library

Main job of this project is actually nothing. It just blinks led and sends a constant string throught the UART in a while loop. Also it has UART Receive interrupt routine to check if we can get interrupts or not.

## How to Build

More detailed steps can be found in the GHA yml file but basically;

1. You need to setup the environment

```
cd ..
./setup_environment.sh
```

2. To build firmware

```
./build.sh -h     // to see help menu
./build.sh        // to compile the project in Debug mode and boot from RAM option
./build.sh -r     // to compile the project in Release mode and boot from RAM option
./build.sh -r -f  // to compile the project in Release mode and boot from FLASH option
./build.sh -f     // to compile the project in Debug mode and boot from FLASH option
```


## How to Flash

```
./flash.sh <flash_address>  // use this if you have RAM application
./flash.sh                  // use this if you have FLASH application
```


## How to Use

Open a serial terminal to see messages coming from the firmware


## Notes

Linker script seperation is the key point of this project.
