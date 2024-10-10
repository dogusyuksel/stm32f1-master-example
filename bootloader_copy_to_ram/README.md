# STM32F103 Bootloader Example To Copy Flash into RAM with HAL Library

Main job of this project is getting a "jump" address from serial port in hex format (like 800c800) and jumps to that address.
If the project is compiled with "-f" option, it will be jump to specified address directly into the flash
If it is compiled without "-f" option, it means it will be the RAM copy bootloader, than it will copy flash starting from the address that user typed via serial port into the RAM and jumps to the ram.

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
./build.sh        // to compile the project in Debug mode and jumpt to RAM option
./build.sh -r     // to compile the project in Release mode and jumpt to RAM option
./build.sh -r -f  // to compile the project in Release mode and jumpt to FLASH option
./build.sh -f     // to compile the project in Debug mode and jumpt to FLASH option
```


## How to Flash

```
./flash.sh // it will be flashed to the very begginning of the flash because it is bootloader
```


## How to Use

Open a serial terminal to see messages coming from the firmware and type the address to be jumped


## Notes

Linker script seperation is the key point of this project.
