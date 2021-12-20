/*
; sudoku.h
;
; Course: HRY411 - Embedded Microprocessor Systems, Winter Semester of acad. year 2021-22
;
; Semester project - Sudoku on Atmel AVR
; Students             : Kallinteris Andreas, 2017030066
;                      : Lioudakis Emmanouil, 2018030020
; Device in simulation : ATmega16 (WARNING: If another device is used, some registers may differ a bit)
; Device in STK500     : ATmega16L - fully compatible with ATmega16
; IDE used             : Microchip Studio 7
; CPU freq             : 10 MHz, using an oscillator in STK500 (If changed, things such as timer init. value and UBRR for the serial port should change too)
*/

#include <avr/io.h>
#include <avr/interrupt.h>  /* To use interrupt handlers */
#include <avr/iom16.h>      /* Definition for UDR on ATmega16 */
#include <stdbool.h>        /* To use bool flags */
#include <avr/pgmspace.h>   /* To be able to use the flash memory */

#define F_CPU 10000000             /* CPU Clock = 10MHz */
#define BAUD 9600                  /* Data rate of 9600 BAUD for USART */
#define MYUBRR ((F_CPU/16/BAUD)-1) /* Calculate UBRR's value */

//#define SIMULATION_MODE  /* If defined, simulation mode is enabled. USART input is taken from TCNT2 and USART output is redirected to TCNT0 */

/************************************************************************/
/* Global variables declaration                                         */
/************************************************************************/

/* Struct to keep the program flags */
struct flags_struct {
	bool received_A  : 1; /* Shows if we have received A */
	bool received_B  : 1; /* Shows if we have received B */
	bool received_C  : 1; /* Shows if we have received C */
	bool received_D  : 1; /* Shows if we have received D */
	bool received_K  : 1; /* Shows if we have received K */
	bool received_N  : 1; /* Shows if we have received N */
	bool received_O  : 1; /* Shows if we have received O */
	bool received_P  : 1; /* Shows if we have received P */
	bool received_S  : 1; /* Shows if we have received S */
	bool received_T  : 1; /* Shows if we have received T */
	bool received_CR : 1; /* Shows if we have received <CR> (added after milestone 1) */
	bool transmit_barrier : 1;  /* TRUE : Transmitting data is permitted
	                               FALSE: Break transmitting data */
};

/* Union to keep the counter of the cell received/transmitted on X dimension */
/* At a given moment, only one of them is used (when we send, we don't receive a new value and vice versa) */
union counters_X {
	uint8_t received_X;     /* The X index of the character received */
	uint8_t sent_counter_X; /* The X index of the character to send */
};

/* Union to keep the counter of the cell received/transmitted on Y dimension */
/* At a given moment, only one of them is used (when we send, we don't receive a new value and vice versa) */
union counters_Y {
	uint8_t received_Y;     /* The Y index of the character received */
	uint8_t sent_counter_Y; /* The Y index of the character to send */
};

uint8_t args_counter; /* A counter useful to know if we are receiving X,Y or VALUE after the "N"  or X, Y after the "D" */

/* Store the LookUp Table for the LED progress bar in flash memory (based on Application Note AVR4027, Section 3.5 (Tip #5 – Constants in program space)) */
const uint8_t led_bar_LUT[82] PROGMEM = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00};
/* 0-9   cells completed -> All LEDS off (PORTA = 0xFF) */
/* 10-19 cells completed -> LED0         (PORTA = 0xFE) */
/* 20-29 cells completed -> LED1-0       (PORTA = 0xFC) */
/* 30-39 cells completed -> LED2-0       (PORTA = 0xF8) */
/* 40-49 cells completed -> LED3-0       (PORTA = 0xF0) */
/* 50-59 cells completed -> LED4-0       (PORTA = 0xE0) */
/* 60-69 cells completed -> LED5-0       (PORTA = 0xC0) */
/* 70-79 cells completed -> LED6-0       (PORTA = 0x80) */
/* 80-81 cells completed -> LED7-0       (PORTA = 0x00) */

/************************************************************************/
/* Function declarations                                                */
/* The functions included in main.cpp                                   */
/************************************************************************/
ISR(TIMER1_COMPA_vect);
ISR (USART_RXC_vect);
void USART_Transmit(unsigned char data);
void init(void);
void USART_init(unsigned int ubrr);
void portA_init(void);
void timer1_init(void);
void clear_char_flags(void);
void clear(void);
void send_done(void);
void send_cell(uint8_t bcd_x, uint8_t bcd_y);
void send_ok(void);

