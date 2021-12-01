/*
; main.c
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

#include "Sudoku.h"
#include "kallinteris.hpp"
#include "LUT.hpp"
#include "sudoku.cpp"

/* An instance of the sudoku class */
sudoku base_board;

/* An instance of these struct-unions */
struct flags_struct flg;
union counters_X cnt_X;
union counters_Y cnt_Y;


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
	while ( !( UCSRA & (1<<UDRE)) )
		;
	
	/* Ready to transmit: Put the data into buffer, send the data */
	#ifdef SIMULATION_MODE
		TCNT0 = data;
	#endif
	#ifndef SIMULATION_MODE
		UDR = data; 
	#endif
		
}

/************************************************************************/
/* send_ok (C equivalent of send_ok in Lab3, but here a function        */
/*             is used to transmit)                                     */
/* Function to send an OK<CR><LF> message                               */
/************************************************************************/
/* May convert to macro (to reduce function calls) */
void send_ok (void){
	
	/* Clean the flag registers, because we ended decoding this command */
	clear_char_flags();
	
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
	clear_char_flags();
	
	/* Find the value, and convert it to ASCII */
	/* Subtract 1 from x and y, because in the assignment X and Y start from 1, but
	     in C, array positions start from 0. */
	uint8_t value_to_send =  base_board.get_cell(bcd_y, bcd_x) + 0x30;
	
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
	/* CLEAN FLAG REGS, OR NOT */
	clear_char_flags();
	
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
	/* Initialize the grid (and update the cells counter) */
	base_board.clear();
	
	/* Turn off the LEDs (0 cells completed) */
	PORTA = 0xFF;

}

/************************************************************************/
/* The interrupt service routine for the UART receive event             */
/* (C equivalent of USART_RXC in Lab3)                                  */
/************************************************************************/
ISR (USART_RXC_vect){
	unsigned char received_char = UDR;
	
	#ifdef SIMULATION_MODE
		received_char = UDR;  /* Read UDR one more time, to consume RXC flag */
		received_char = TCNT2;
	#endif
	
	switch(received_char){
		case 0x41:
			flg.received_A = true;
			break;
		case 0x54:
			flg.received_T = true;
			break;
		case 0x43:
			flg.received_C = true;
			clear();
			break;
		case 0x4E:
			flg.received_N = true;
			args_counter = 0;
			break;
		case 0x50:
			flg.received_P = true;
			break;
		case 0x53:
			flg.received_S = true;
			break;
		case 0x42:
			flg.received_B = true;
			break;
		case 0x44:
			flg.received_D = true;
			args_counter = 0;
			break;
		case 0x4F:
			flg.received_O = true;
			break;
		case 0x4B:
			flg.received_K = true;
			break;
		case 0x0D:
			break;
		case 0x0A:
			/* If "<LF>", process and return
			   <LF> is the last character of each command, so it 
				   is time to execute the received command */
		
			if( (flg.received_A && flg.received_T) || (flg.received_C) || (flg.received_N) ){
				/* If command was AT or C or N, send OK */
				send_ok();
			}
			else if(flg.received_P){
				/* If command was P,start solving the sudoku after sending OK */
				//flg.solving_barrier = true;
				base_board.set_solving_barrier(true);
				send_ok();
			}
			else if(flg.received_S){
				/* If command was S, start sending values */
				flg.transmit_barrier = true; /* Set the transmit barrier, to be able to send the next cells */
				/* Send the first cell */
				cnt_X.sent_counter_X = 1;
				cnt_Y.sent_counter_Y = 1;
				send_cell(cnt_X.sent_counter_X, cnt_Y.sent_counter_Y);
			}
			else if(flg.received_T){
				/* If command was T, send next value */
				if(flg.transmit_barrier){
					cnt_X.sent_counter_X++;
					if(cnt_X.sent_counter_X == 10){
						cnt_X.sent_counter_X = 1;
						cnt_Y.sent_counter_Y++;
					}
					send_cell(cnt_X.sent_counter_X, cnt_Y.sent_counter_Y);
				}
				/* Of course, if break_transmission=0(after a B command), we will not send something back to the computer */
			
			}
			else if(flg.received_B){
				/* If command was B, stop calculations or sending results and send OK */
				base_board.set_solving_barrier(false);
				flg.transmit_barrier = false;
				send_ok();
			}
			else if(flg.received_D){
				/* If command was D, send cell contents (they are already prepared when we received Y) */
				send_cell(cnt_X.sent_counter_X, cnt_Y.sent_counter_Y);
			}
			else if(flg.received_O && flg.received_K){
				/* If command was OK, this means that the computer has received all the values */
				/* TODO */ /*Is something needed here ? */
			}
			
			break;
			
		default:
			/* Received numbers (Commands N and D from computer) */
		
			if(flg.received_N){ /* If command was N, the first time read X, the second time read Y, the third time read VALUE and store it */
				if(args_counter == 0){                 /* First argument, X */
					cnt_X.received_X = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
					args_counter++;
				}
				else if(args_counter == 1){            /* Second argument, Y */
					cnt_Y.received_Y = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
					args_counter++;
				}
				else if(args_counter == 2){            /* Third argument, VALUE */
					received_char = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
					base_board.set_cell(cnt_Y.received_Y, cnt_X.received_X, received_char);
					//grid[cnt_Y.received_Y-1][cnt_X.received_X-1] = received_char; /* Subtract 1 because in C, array index starts from 0, but in our UART protocol, it starts from 1 */
					//num_cnt++; /* Note that we received one more value */
					PORTA = pgm_read_byte(&led_bar_LUT[base_board.get_solved_cell_counter()]);
				}
			}
			else if(flg.received_D){ /* If command was D, the first time read X, the second time read Y, the third time prepare VALUE to be sent */
				 if(args_counter == 0){                 /* First argument, X */
					 cnt_X.received_X = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */
					 args_counter++;
				 }
				 else if(args_counter == 1){            /* Second argument, Y */
					 cnt_Y.received_Y = received_char & 0x0F; /* Convert from ASCII to BCD, using the appropriate mask */	 
				 }
			}
		
		
		
	}
		
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void clear_char_flags(void){
	flg.received_A = false;
	flg.received_B = false;
	flg.received_C = false;
	flg.received_D = false;
	flg.received_K = false;
	flg.received_N = false;
	flg.received_O = false;
	flg.received_P = false;
	flg.received_S = false;
	flg.received_T = false;	
}

/************************************************************************/
/* init                                                                 */
/* Function to initialize global variables, ports, counters and USART   */
/************************************************************************/
void init(void){
	
	/* Initialize the global variables for the serial port */
	args_counter = 0;
	cnt_X.received_X = 0x00;
	cnt_Y.received_Y = 0x00;
	
	cnt_X.sent_counter_X = 1;
	cnt_Y.sent_counter_Y = 1;
	
	clear_char_flags();
	base_board.set_solving_barrier(false); //flg.solving_barrier = false;
	flg.transmit_barrier = false;
	
	
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
	/* Initialize global variables, PortA and serial port */
	init();
	
	/* Enable global interrupts support */
	sei();
	
	/* Do an infinite loop */
	while (1) { 
		if(base_board.get_solving_barrier()){
			if(base_board.solve()){
				base_board.set_solving_barrier(false);
				send_done();
			}
		}
	}
		
	
}




