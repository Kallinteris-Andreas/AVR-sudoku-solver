/*
; main.c
;
; Course: HRY411 - Embedded Microprocessor Systems, Winter Semester of acad. year 2021-22
;
; Semester project - Sudoku on Atmel AVR
; Students             : Lioudakis Emmanouil, 2018030020
; Device in simulation : ATmega16 (WARNING: If another device is used, some registers may differ a bit)
; Device in STK500     : ATmega16L - fully compatible with ATmega16
; IDE used             : Microchip Studio 7
; CPU freq             : 10 MHz, using an oscillator in STK500 (If changed, things such as timer init. value and UBRR for the serial port should change too)
*/

#include <avr/io.h>
#include <avr/interrupt.h>  //To use interrupt handlers
#include <avr/iom16.h>      //Definition for UDR

#define F_CPU 10000000             // CPU Clock = 10MHz
#define BAUD 9600                  // Data rate of 9600 BAUD for USART
#define MYUBRR ((F_CPU/16/BAUD)-1) // Calculate UBRR's value

/************************************************************************/
/* Macros TODO: Add more comments                                       */
/************************************************************************/
#define exists_in_row(row, value) \
{ \
	(grid[row][0] == value || grid[row][1] == value || grid[row][2] == value \
	|| grid[row][3] == value || grid[row][4] == value || grid[row][5] == value \
	|| grid[row][6] == value || grid[row][7] == value || grid[row][8] ==value); \
}

#define exists_in_column(column, value) \
{ \
	(grid[0][column] == value || grid[1][column] == value || grid[2][column] == value \
	|| grid[3][column] == value || grid[4][column] == value || grid[5][column] == value \
	|| grid[6][column] == value || grid[7][column] == value || grid[8][column] ==value); \
}

#define exists_in_box(row, column, value) \
{ \
	int row_of_box = row - row%3; \
	int column_of_box = column - column %3; \
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
/* Global variables                                                     */
/************************************************************************/
volatile uint8_t flag_reg;  /* -flag_reg bit0 shows if we have received A
                               -flag_reg bit1 shows if we have received T
                               -flag_reg bit2 shows if we have received C
                               -flag_reg bit3 shows if we have received N
                               -flag_reg bit4 shows if we have received P
							   -flag_reg bit5 shows if we have received S
							   -flag_reg bit6 shows if we have received T
							   -flag_reg bit7 is useless */
volatile uint8_t num_cnt;   /* A counter for the numbers we have in the grid */


volatile uint8_t args_counter; /* A counter useful to know if we are receiving X,Y or VALUE after the "N" */

volatile uint8_t received_X;
volatile uint8_t received_Y; /* Useful to keep X and Y to write the VALUE to the grid in the 3rd ISR call after "N" */

volatile uint8_t start_game; /* Used for the P command */
volatile uint8_t break_game; /* Used for the B command */ /*Maybe only one of them is useful, FOR NOW keep both */

volatile uint8_t debug_flags; /* TODO: explain */


/* A function to keep the sudoku in SRAM */
volatile uint8_t grid[9][9];

/************************************************************************/
/* DEFINE FUNCTIONS                                                     */
/************************************************************************/
// void function();

/************************************************************************/
/* USART_init (C equivalent of usart_init in Lab3)                      */
/* Function to initialize the serial port                               */
/************************************************************************/
void USART_init (unsigned int ubrr){
	/* Configure baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	
	/* Set Asynchronous Normal Speed Mode (pg 164/357)*/
	UCSRA = (0<<U2X);
	
	/* Configure UCSRB (pg 165/357)
	 RXEN  = 1 (Enable receiver)
	 TXEN  = 1 (Enable transmitter)
	 RXCIE = 1 (Enable RX complete interrupt)
	 UCSZ2 = 0 (the bits 1-0 are in the UCSRC register)-(frame format) 
	 */
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	
	
	/* Configure UCSRC (pg 166/357)
	 URSEL   = 1 (to write UCSRC and not UBRRH)
	 UMSEL   = 0 (Asynchronous Operation Mode)
	 UPM1:0  = 00 (Parity Configuration-Disable parity mode)
	 USBS    = 0 (Frame format: 1 stop bit)
	 UCSZ1:0 = 11 (Frame format: Character size 8 bits) --- UCSZ2:0 = 011
	 */
	UCSRC = (1<<URSEL)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}

/************************************************************************/
/* ports_init                                                           */
/* Function to initialize Port A for the LEDs                           */
/************************************************************************/
void portA_init (void){
	
	/* PortA - which segments will light */
	DDRA = 0xFF;   /* PortA will be output */
	PORTA = 0xFF;  /* Turn all the segments off */
	
	/*PortD - serial port
	 RxD is Port D, Bit 0
	 TxD is Port D, Bit 1

	 We do nothing, because regardless of the values of the Data Direction Register
	   for Port D, if USART transmitter is enabled, RxD is configured as input and
	   TxD is configured as output automatically (pg 64/357 of the manual)
	*/
}

/************************************************************************/
/* updateLEDS                                                           */
/* Function to show the solving progress to LEDS7-0                     */
/*    (LEDS on STK are Common Anode)                                    */
/************************************************************************/
/* May convert to macro (to reduce function calls) and ORGANIZE decisions, to use less CPU cycles*/
void updateLEDS (uint8_t completed_cells){
	uint8_t tmp = completed_cells/10;
		
	if(tmp == 0){
		PORTA = 0xFF; /* 0-9 completed -> All LEDS off */
	}
	if(tmp == 1){
		PORTA = 0xFE; /* 10+ completed -> LED0   */
	}
	else if(tmp == 2){
		PORTA = 0xFC; /* 20+ completed -> LED1-0 */
	}
	else if(tmp == 3){
		PORTA = 0xF8; /* 30+ completed -> LED2-0 */
	}
	else if(tmp == 4){
		PORTA = 0xF0; /* 40+ completed -> LED3-0 */
	}
	else if(tmp == 5){
		PORTA = 0xE0; /* 50+ completed -> LED4-0 */
	}
	else if(tmp == 6){
		PORTA = 0xC0; /* 60+ completed -> LED5-0 */
	}
	else if(tmp == 7){
		PORTA = 0x80; /* 70+ completed -> LED6-0 */
	}
	else if(tmp == 8){
		PORTA = 0x00; /* 80+ completed -> LED7-0 */
	}	
}

/************************************************************************/
/* Pseudocode for the backtracking algorithm implementation             */
/************************************************************************/
/* TODO */

/************************************************************************/
/* USART_Transmit                                                       */
/* Function to transmit a character to the serial port.                 */
/* Based on AVR manual, pg. 151/357                                     */
/*                                                                      */
/* Polling is needed to ensure that UDR is available to write data      */
/************************************************************************/
void USART_Transmit(unsigned char data){
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );
	
	/* Ready to transmit: Put the data into buffer, send the data */
	UDR = data; 
	
}

/************************************************************************/
/* send_ok (C equivalent of send_ok in Lab3, but here a function        */
/*             is used to transmit)                                     */
/* Function to send an OK<CR><LF> message                               */
/************************************************************************/
/* May convert to macro (to reduce function calls) */
void send_ok (void){
	
	/* Clean the flag_register, because we ended decoding this command */
	flag_reg = 0x00;
	
	/* Send an OK<CR><LF> message */
	USART_Transmit(0x4F); /* Send "O" */
	USART_Transmit(0x4B); /* Send "K" */
	USART_Transmit(0x0D); /* Send "<CR>" */
	USART_Transmit(0x0A); /* Send "<LF>" */
	
}


/************************************************************************/
/* clear                                                                */
/* Function to initialize all the cells of the grid to 0 and turn off   */
/*   all the LEDs                                                       */
/************************************************************************/
void clear(void){
	/* Initialize the grid */
	for(int i=0; i<9; i++){
		for(int j=0; j<9; j++){
			grid[i][j] = 0;
		}
	}
	
	/* Turn off the LEDs */
	PORTA = 0xFF;
}

/************************************************************************/
/* The interrupt service routine for the UART receive event             */
/* (C equivalent of USART_RXC in Lab3)                                  */
/************************************************************************/
ISR (USART_RXC_vect){
	unsigned char received_char = UDR;
	
	if (received_char == 0x41){        //A
		/* If "A", flag and return */
		flag_reg = flag_reg | 0x01;
	} 
	else if (received_char == 0x54){   //T
		/* If "T", flag and return */ 
		flag_reg = flag_reg | 0x02;
	}
	else if (received_char == 0x43){   //C
		/* If "C", flag and return */  /* TODO: Clear maybe will be done after LF */
		flag_reg = flag_reg | 0x04;
		clear();
	}
	else if (received_char == 0x4E){   //N
		/* If "N", flag, initialize the counter and return */
		flag_reg = flag_reg | 0x08;
		args_counter = 0;
	}
	else if (received_char == 0x50){   //P
		/* If "P", flag and return. Play should start after CR-LF */
		flag_reg = flag_reg | 0x10;
	}
	else if (received_char == 0x0D){   //<CR>
		/* If "<CR>", simply return, no flag is needed */
	}
	else if (received_char == 0x0A){   //<LF>
		/* If "<LF>", process and return
		   <LF> is the last character of each command, so it 
		       is time to execute the received command */
		
		if(flag_reg == 0x03){
			/* If command was AT, send OK */
			send_ok();
		}
		else if(flag_reg == 0x04){
			/* If command was C, send OK */
			/* Clear has already been done (when we received C), simply send OK */
			send_ok();
		}
		else if(flag_reg == 0x08){
			/* If command was N , send OK (the value is already stored in the grid */
			args_counter = 0;
			send_ok();
		}
		else if(flag_reg == 0x10){
			/* If command was P, send OK and start solving the sudoku */
			start_game = 1;
			break_game = 0;
		}
		else if(flag_reg == 0x20){
			/* If command was S, start sending values */
			/* TODO */
		}
		else if(flag_reg == 0x40){
			/* If command was T, ... */
			/* TODO */
		}
		
		
	}
	else{ //Received numbers
		
		if(flag_reg == 0x08){ /* If command was N , the first time read X, the second time read Y, the third time read VALUE and store it */
			if(args_counter == 0){                 /* First argument, X */
				received_X = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
				args_counter++;
			}
			else if(args_counter == 1){            /* Second argument, Y */
				received_Y = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
				args_counter++;
			}
			else if(args_counter == 2){            /* Third argument, VALUE */
				received_char = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
				grid[received_X-1][received_Y-1] = received_char; /* Subtract 1 because in C, array index starts from 0, but in our UART protocol, it starts from 1 */
				args_counter = 0; /* TODO: Maybe not needed */
				num_cnt++; /* Note that we received one more value */ /* TODO: update the LED */
			}
		}
	}
		
}

uint8_t solve_sudoku(){
	// Add somewhere a condition to stop solving
	for(int i=0; i<9; i++){
		for(int j=0; j<9; j++){
			if(grid[i][j] == 0){
				for(int value=1; value<=9; value++){
					if((!(exists_in_row(i, value))) && (!(exists_in_column(j, value))) && (!(exists_in_box(i, j, value)))){
						grid[i][j] = value;
						if(solve_sudoku()){
							return 1;
						}
						else{
						grid[i][j] = 0;
						}
					}
				}
				return 0; /*backtrack */
			}
		}
	}
	return 1;
}



/************************************************************************/
/* init                                                                 */
/* Function to initialize global variables, ports, counters and USART   */
/************************************************************************/
void init(void){
	
	/* Initialize the global variables for the serial port */
	flag_reg = 0x00;
	num_cnt = 0;
	start_game = 0;
	break_game = 0;
	
	/* */
	args_counter = 0;
	
	/* Initialize the Ports A and C */
	portA_init();
	
	/* Initialize (clear) the screen (the data of the array "digits") */
	//clear_screen();
	
	/* Initialize the serial port */
	USART_init(MYUBRR);
}

/************************************************************************/
/* main function                                                        */
/* The main part of the program                                         */
/************************************************************************/
int main(void){	
	/* Initialize global variables, ports, counter0 and serial port */
	init();
	
	/* Enable global interrupts */
	sei();
	
	/* Do an infinite loop */
	while (1) { 
		if(start_game == 1 && break_game == 0){
			//call solve 
			
			solve_sudoku();
			// if a debug character is received, the solving will stop
			
			// TODO: When ISR returns, what will happen? We want it to stop
		}
	
	}
		
	
}




