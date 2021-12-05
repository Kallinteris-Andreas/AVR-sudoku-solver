# Semester project - Sudoku on Atmel AVR
#### Course
HRY411 - Embedded Microprocessor Systems, Winter Semester of acad. year 2021-22 
#### Students            
Kallinteris Andreas (2017030066), Lioudakis Emmanouil (2018030020)
#### Device in simulation 
ATmega16 (WARNING: If another device is used, some registers may differ a bit)
#### Device on STK500     
ATmega16L - fully compatible with ATmega16
#### IDE used
Microchip Studio 7
#### CPU frequency
10 MHz, using an oscillator on STK500 (If changed, things such as timer init. value and UBRR for the serial port should change too)

#### Description of the submitted files
The folder "CODE" contains the code developed for the project
team5milestone.elf is the file download to the STK (produced by compiling the code)
The folder "STIMULI_FILES" contains the stimuli files used to test the functionality of the program
The folder "PUTTY_LOGS" contains the PuTTY logs of three tests on real hardware (more info in the report)

# build instructions

## using gnu-toolchain on linux

```
git clone https://github.com/apcountryman/toolchain-avr-gcc.git
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-avr-gcc/toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && make
```



## using microchip studio

make sure to use gnu c++ compiler (g++) and provide the flags `-DAVR --std=c++17`
