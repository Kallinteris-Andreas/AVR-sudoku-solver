# Semester project - Sudoku on Atmel AVR
### Course
ACE411 - Embedded Microprocessor Systems, Winter Semester of acad. year 2021-22 
### Authors
Kallinteris Andreas, Lioudakis Emmanouil
### Target Hardware
[ATmega16](https://www.microchip.com/en-us/product/atmega16) (WARNING: If another device is used, some registers may differ a bit)
[ATmega16L](https://www.microchip.com/en-us/product/atmega16) - binary compatible with ATmega16
10 MHz, using an oscillator on STK500 (If changed, `DF_CLK` should be changed also)

# Description of the submitted files (WIP)

# build instructions

## using gnu-toolchain on linux

```
git clone https://github.com/apcountryman/toolchain-avr-gcc.git
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-avr-gcc/toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && make
```

## using microchip studio 7
make sure to use gnu c++ compiler (g++) and provide the flags `-DAVR --std=c++17`
