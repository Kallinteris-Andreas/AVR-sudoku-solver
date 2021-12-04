/*
 * IncFile1.h
 *
 * Created: 28/11/2021 8:05:49 μμ
 *  Author: lioud
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#include <avr/io.h>
#include <avr/interrupt.h>  //To use interrupt handlers
#include <avr/iom16.h>      //Definition for UDR
#include <stdbool.h>        //To use bool flags
#include <avr/pgmspace.h>   //To be able to use the flash memory

#define F_CPU 10000000             // CPU Clock = 10MHz
#define BAUD 9600                  // Data rate of 9600 BAUD for USART
#define MYUBRR ((F_CPU/16/BAUD)-1) // Calculate UBRR's value

#define SIMULATION_MODE

/************************************************************************/
/* Macros TODO: Add more comments                                       */
/************************************************************************/
#define EXISTS_IN_ROW(row, value) \
{ \
	(grid[row][0] == value || grid[row][1] == value || grid[row][2] == value \
	|| grid[row][3] == value || grid[row][4] == value || grid[row][5] == value \
	|| grid[row][6] == value || grid[row][7] == value || grid[row][8] ==value); \
}

#define EXISTS_IN_COLUMN(column, value) \
{ \
	(grid[0][column] == value || grid[1][column] == value || grid[2][column] == value \
	|| grid[3][column] == value || grid[4][column] == value || grid[5][column] == value \
	|| grid[6][column] == value || grid[7][column] == value || grid[8][column] ==value); \
}

#define EXISTS_IN_BOX(row, column, value) \
{ \
	uint8_t row_of_box = row - row%3; \
	uint8_t column_of_box = column - column %3; \
	(grid[row_of_box][column_of_box] == value || \
	grid[row_of_box][column_of_box + 1] == value || \
	grid[row_of_box][column_of_box + 2] == value || \
	grid[row_of_box + 1][column_of_box] == value || \
	grid[row_of_box + 1][column_of_box + 1] == value || \
	grid[row_of_box + 1][column_of_box + 2] == value || \
	grid[row_of_box + 2][column_of_box] == value || \
	grid[row_of_box + 2][column_of_box + 1] == value || \
	grid[row_of_box + 2][column_of_box + 2] == value); \
}

/************************************************************************/
/* Global variables declaration                                         */
/************************************************************************/

struct flags_struct {
	bool received_A; /* Shows if we have received A */
	bool received_B; /* Shows if we have received B */
	bool received_C; /* Shows if we have received C */
	bool received_D; /* Shows if we have received D */
	bool received_K; /* Shows if we have received K */
	bool received_N; /* Shows if we have received N */
	bool received_O; /* Shows if we have received O */
	bool received_P; /* Shows if we have received P */
	bool received_S; /* Shows if we have received S */
	bool received_T; /* Shows if we have received T */

	bool solving_barrier;    /* TRUE : Solving is permitted
	                            FALSE: Break solving */
	bool transmit_barrier;   /* TRUE : Transmitting data is permitted
	                            FALSE: Break transmitting data */
};

union counters_X {
	uint8_t received_X;
	uint8_t sent_counter_X;
};

union counters_Y {
	uint8_t received_Y;
	uint8_t sent_counter_Y;
};

uint8_t num_cnt;   /* A counter for the numbers we have in the grid (not the zeros) */
uint8_t args_counter; /* A counter useful to know if we are receiving X,Y or VALUE after the "N" */
uint8_t grid[9][9]; /* A global variable to keep the sudoku in SRAM */

/* Store the LookUp Table for the LED progress bar in flash memory (based on Application Note AVR4027, Section 3.5 (Tip #5 – Constants in program space) */
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
/* TODO 28-11 Add all the functions of main file                        */
/************************************************************************/
void clear_char_flags(void);



#endif /* INCFILE1_H_ */