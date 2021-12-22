# Semester project - Sudoku on Atmel AVR
### Course
ACE411 - Embedded Microprocessor Systems, Winter Semester of acad. year 2021-22 
### Authors
Kallinteris Andreas (2017030066), Lioudakis Emmanouil (2018030020)
### Target Hardware
[ATmega16](https://www.microchip.com/en-us/product/atmega16) (WARNING: If another device is used, some registers may differ a bit)
[ATmega16L](https://www.microchip.com/en-us/product/atmega16) - binary compatible with ATmega16
10 MHz, using an oscillator on STK500 (If changed, `DF_CLK` should be changed also in makefile)

# Description of the submitted files
The folder "CODE" contains the code developed for the project. 
The folder "STIMULI_FILES" contains the stimuli files used to test the functionality of the program.
The folder "PUTTY_LOGS" contains the PuTTY logs of three tests on real hardware (more info in the report).
The folder "PERFORMANCE_ANALYSIS" contains a simple program that measures the solving time of a large number of boards and exports statistic data
    about the performance of the sudoku solver.
The file "fuses_config.conf" contains the fuses configuration of the STK.
"team5final.elf" is the file to download to the STK (produced by compiling the code with Microchip Studio).
"main12.elf" is the preferred file to use, because it has been compiled on linux system, using ver. 11 of g++ (while Microchip studio uses g++ 5.4), 
therefore it is more optimised by the compiler. When using this, the mean solving time at ultra level is 0.45sec faster than team5final.elf 

# build instructions

## using gnu-toolchain on linux

```
git clone https://github.com/apcountryman/toolchain-avr-gcc.git
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-avr-gcc/toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && make
```

## using microchip studio 7
make sure to use gnu c++ compiler (g++) and provide the flags `-DAVR --std=c++17`
