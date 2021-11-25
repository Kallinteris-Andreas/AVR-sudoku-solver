/*
; main.c
;
; Course: HRY411 - Embedded Microprocessor Systems, Winter Semester of acad. year 2021-22
;
; Semester project - Sudoku on Atmel AVR
; Students             : Lioudakis Emmanouil, 2018030020
;                      : Kallinteris Andreas, 201   ************************************ /* TODO: Update AM 
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
/* updateLEDS                                                           */
/* MACRO to show the solving progress to LEDS7-0                        */
/*    (LEDS on STK are Common Anode)                                    */
/* TODO 25/11: MAY CONVERT TO LOOKUP TABLE, as prof. Dollas suggested   */
/************************************************************************/
#define updateLEDS(completed_cells) \
{ \
	/* Use switch-case, as suggested in AVR4027 Section 4.4 (TODO: needs optimization flag -O3) */ \
	switch(completed_cells/10){ \
		case 0: \
			PORTA = 0xFF; /* 0-9 completed -> All LEDS off */ \
			break; \
		case 1: \
			PORTA = 0xFE; /* 10+ completed -> LED0   */ \
			break; \
		case 2: \
			PORTA = 0xFC; /* 20+ completed -> LED1-0 */ \
			break; \
		case 3: \
			PORTA = 0xF8; /* 30+ completed -> LED2-0 */ \
			break; \
		case 4: \
			PORTA = 0xF0; /* 40+ completed -> LED3-0 */ \
			break; \
		case 5: \
			PORTA = 0xE0; /* 50+ completed -> LED4-0 */ \
			break; \
		case 6: \
			PORTA = 0xC0; /* 60+ completed -> LED5-0 */ \
			break; \
		case 7: \
			PORTA = 0x80; /* 70+ completed -> LED6-0 */ \
			break; \
		case 8: \
			PORTA = 0x00; /* 80+ completed -> LED7-0 */ \
			break; \
	} \
}

/************************************************************************/
/* Global variables                                                     */
/************************************************************************/
volatile uint8_t flag_reg_A;  /* -flag_reg_A bit0 shows if we have received A
                                 -flag_reg_A bit1 shows if we have received T
                                 -flag_reg_A bit2 shows if we have received C
                                 -flag_reg_A bit3 shows if we have received N
                                 -flag_reg_A bit4 shows if we have received P
							     -flag_reg_A bit5 shows if we have received S
							     -flag_reg_A bit6 shows if we have received T
							     -flag_reg_A bit7 is useless */

volatile uint8_t flag_reg_B;  /* -flag_reg_B bit0 shows if we have received O
                                 -flag_reg_B bit1 shows if we have received K
                                 -flag_reg_B bit2 shows if we have received B
                                 -flag_reg_B bit3 shows if we have received D
							     -flag_reg_B upper nibble is useless */


volatile uint8_t num_cnt;   /* A counter for the numbers we have in the grid (not the zeros) */


volatile uint8_t args_counter; /* A counter useful to know if we are receiving X,Y or VALUE after the "N" */

volatile uint8_t received_X;
volatile uint8_t received_Y; /* Useful to keep X and Y to write the VALUE to the grid in the 3rd ISR call after "N" */
/* Could use only 1 8-bit variable and separate the two nibbles, but it would cost more time (we have enough memory) */

volatile uint8_t start_game; /* Used for the P command */
volatile uint8_t break_game; /* Used for the B command */ /*Maybe only one of them is useful, FOR NOW keep both */

/* A global variable to keep the sudoku in SRAM */
volatile uint8_t grid[9][9];

//volatile uint8_t value_to_send; /* Used to send back the grid contents in command D */ /*TODO: May BE USED for more commands */

volatile uint8_t sent_counter_X; /* Used for commands S and T, to send the cells back to the computer */
volatile uint8_t sent_counter_Y;

// volatile uint8_t start_sending; /* Flag to start sending, in case of S command */ /* FOR NOW, not needed */

/************************************************************************/
/* DEFINE FUNCTIONS (like a C header file)                              */
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
	//UDR = data; 
	TCNT1L = data; /* instead of "UDR = data;" for testing */
	
}

/************************************************************************/
/* send_ok (C equivalent of send_ok in Lab3, but here a function        */
/*             is used to transmit)                                     */
/* Function to send an OK<CR><LF> message                               */
/************************************************************************/
/* May convert to macro (to reduce function calls) */
void send_ok (void){
	
	/* Clean the flag registers, because we ended decoding this command */
	flag_reg_A = 0x00;
	flag_reg_B = 0x00;
	
	/* Send an OK<CR><LF> message */
	USART_Transmit(0x4F); /* Send "O" */
	USART_Transmit(0x4B); /* Send "K" */
	USART_Transmit(0x0D); /* Send "<CR>" */
	USART_Transmit(0x0A); /* Send "<LF>" */
	
}

/************************************************************************/
/* send_cell                                                            */
/************************************************************************/
void send_cell (uint8_t bcd_x, uint8_t bcd_y){
	/* Clean the flag registers, because we ended decoding this command */
	flag_reg_A = 0x00;
	flag_reg_B = 0x00;
	
	/* Find the value, and convert it to ASCII */
	uint8_t value_to_send = grid[bcd_x - 1][bcd_y - 1] + 0x30;
	
	/* Send the cell data */
	USART_Transmit(0x4E);          // N
	USART_Transmit(bcd_x + 0x30);  // <X>
	USART_Transmit(bcd_y + 0x30);  // <Y>
	USART_Transmit(value_to_send); // <VALUE>
	USART_Transmit(0x0D);          // <CR>
	USART_Transmit(0x0A);          // <LF>
}

/************************************************************************/
/* send_done                                                            */
/************************************************************************/
void send_done (void){
	/* Send a D<CR><LF> message */  /* Clear flag regs is not needed here */
	USART_Transmit(0x44); // D
	USART_Transmit(0x0D); // <CR>
	USART_Transmit(0x0A); // <LF> 
}


/************************************************************************/
/* clear                                                                */
/* Function to initialize all the cells of the grid to 0 and turn off   */
/*   all the LEDs                                                       */
/************************************************************************/
void clear(void){
	/* Initialize the grid */
	for(uint8_t i=0; i<9; i++){
		for(uint8_t j=0; j<9; j++){
			grid[i][j] = 0;
		}
	}
	
	/* Turn off the LEDs (0 cells completed) */
	PORTA = 0xFF;
}

/************************************************************************/
/* The interrupt service routine for the UART receive event             */
/* (C equivalent of USART_RXC in Lab3)                                  */
/************************************************************************/
ISR (USART_RXC_vect){
	unsigned char received_char = UDR;
	
	/* Added for simulation */
	received_char = UDR;  /* Read UDR one more time, to consume RXC flag */
	received_char = TCNT2;
	
	if (received_char == 0x41){        //A
		/* If "A", flag and return */
		flag_reg_A = flag_reg_A | 0x01;
	} 
	else if (received_char == 0x54){   //T
		/* If "T", flag and return */ 
		flag_reg_A = flag_reg_A | 0x02;
	}
	else if (received_char == 0x43){   //C
		/* If "C", flag and return */  /* TODO: Clear maybe will be done after LF */
		flag_reg_A = flag_reg_A | 0x04;
		clear();
	}
	else if (received_char == 0x4E){   //N
		/* If "N", flag, initialize the counter and return */
		flag_reg_A = flag_reg_A | 0x08;
		args_counter = 0;
	}
	else if (received_char == 0x50){   //P
		/* If "P", flag and return. Play should start after CR-LF */
		flag_reg_A = flag_reg_A | 0x10;
	}
	else if(received_char == 0x53){    //S
		/* If "S", flag and return. The first cell will be sent from AVR after receiving LF */
		flag_reg_A = flag_reg_A | 0x20;
	}
	else if(received_char == 0x54){    //T
		/* If "T", flag and return. The next cell will be sent from AVR after receiving LF */
		flag_reg_A = flag_reg_A | 0x40;
	}
	else if(received_char == 0x42){    //B
		/* If "B", flag and return. The solving/sending process should "freeze" after <LF>. */
		flag_reg_B = flag_reg_B | 0x04;
	}
	else if(received_char == 0x44){    //D
		/* If "D", flag initialize the counter to get X and Y in the next calls and return */
		flag_reg_B = flag_reg_B | 0x08;
		args_counter = 0;
	}
	else if(received_char == 0x4F){    //O
		flag_reg_B = flag_reg_B | 0x01;	
	}
	else if(received_char == 0x4B){    //K
		flag_reg_B = flag_reg_B | 0x02;
	}
	else if (received_char == 0x0D){   //<CR>
		/* If "<CR>", simply return, no flag is needed */
	}
	else if (received_char == 0x0A){   //<LF>
		/* If "<LF>", process and return
		   <LF> is the last character of each command, so it 
		       is time to execute the received command */
		
		if(flag_reg_A == 0x03){
			/* If command was AT, send OK */
			send_ok();
		}
		else if(flag_reg_A == 0x04){
			/* If command was C, send OK */
			/* Clear has already been done (when we received "C"), simply send OK */
			send_ok();
		}
		else if(flag_reg_A == 0x08){
			/* If command was N , send OK (the value is already stored in the grid) */
			// args_counter = 0; PROBABLY not needed here
			send_ok();
		}
		else if(flag_reg_A == 0x10){
			/* If command was P,start solving the sudoku after sending OK */
			start_game = 1;
			break_game = 0;
			send_ok();
		}
		else if(flag_reg_A == 0x20){
			/* If command was S, start sending values */
			///////////////////////////////////////start_sending = 1; FOR NOW, not needed
			/* Send the first cell */
			sent_counter_X = 1;
			sent_counter_Y = 1;
			send_cell(sent_counter_X, sent_counter_Y);
		}
		else if(flag_reg_A == 0x40){
			/* If command was T, send next value */
			sent_counter_Y++;
			if(sent_counter_Y == 10){
				sent_counter_Y = 1;
				sent_counter_X++;
			}
			send_cell(sent_counter_X, sent_counter_Y);
			
		}
		else if(flag_reg_B == 0x04){
			/* If command was B, stop calculations or sending results and send OK */
			break_game = 1;
			// TODO: Stop sending results
			send_ok();
		}
		else if(flag_reg_B == 0x08){
			/* If command was D, send cell contents (they are already prepared when we received Y) */
			send_cell(received_X, received_Y);
		}
		else if(flag_reg_B == 0x03){
			/* If command was OK, this means that the computer has received all the values */
			/* TODO */ /*Is something needed here ? */
		}
		
		
	}
	else{ /* Received numbers (Commands N and D from computer) */
		
		if(flag_reg_A == 0x08){ /* If command was N, the first time read X, the second time read Y, the third time read VALUE and store it */
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
				//args_counter = 0; /* TODO: Maybe not needed */
				num_cnt++; /* Note that we received one more value */
				updateLEDS(num_cnt);
			}
		}
		else if(flag_reg_B == 0x08){ /* If command was D, the first time read X, the second time read Y, the third time prepare VALUE to be sent */
			 if(args_counter == 0){                 /* First argument, X */
				 received_X = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
				 args_counter++;
			 }
			 else if(args_counter == 1){            /* Second argument, Y */
				 received_Y = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
				// args_counter = 0; /* TODO: Maybe not needed */	 
			 }
		}
		
		
		
	}
		
}


/************************************************************************/
/* TODO: Pseudocode for the backtracking algorithm implementation       */
/* TODO: Decide if table lookup (said in classroom) is needed for acceleration */
/************************************************************************/
uint8_t solve_sudoku(){	
	// TODO: Add somewhere a condition to stop solving (for the B command)
	for(uint8_t i=0; i<9; i++){
		for(uint8_t j=0; j<9; j++){
			if(grid[i][j] == 0){
				for(uint8_t value=1; value<=9; value++){
					if((!(exists_in_row(i, value))) && (!(exists_in_column(j, value))) && (!(exists_in_box(i, j, value)))){
						grid[i][j] = value;
						num_cnt++; // Increment
						updateLEDS();
						if(solve_sudoku()){
							return 1;
						}
						else{
						grid[i][j] = 0;
						num_cnt--; // Decrement
						updateLEDS();
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
	flag_reg_A = 0x00;
	flag_reg_B = 0x00;
	num_cnt = 0;
	start_game = 0;
	break_game = 0;
	
	/* */
	args_counter = 0;
	received_X = 0x00;
	received_Y = 0x00;
	
	sent_counter_X = 0;
	sent_counter_Y = 0;
	
	/* Configure and initialize PortA */
	portA_init();
	
	/* Initialize memory and PortA */
	clear();
	
	/* Initialize the serial port */
	USART_init(MYUBRR);
}


/************************************************************************/
/* main function                                                        */
/* The main part of the program                                         */
/************************************************************************/
int main(void){	
	/* Initialize global variables, ports and serial port */
	init();
	
	/* Enable global interrupts */
	sei();
	
	/* Do an infinite loop */
	while (1) { 
		if(start_game == 1 && break_game == 0){ //This will not stop solving if receiving "B"
		
			if(solve_sudoku()){
				send_done();
			}
			
			
			
			// if a B command is received, the solving will stop
			
			// TODO: When ISR returns from B, what will happen? We want it to stop
		}
	
	}
		
	
}




