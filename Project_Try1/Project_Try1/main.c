/*
; main.c
;
; Course: HRY411 - Embedded Microprocessor Systems, Winter Semester of acad. year 2021-22
;
; Lab 5 - C code for Atmel AVR
; Student : Lioudakis Emmanouil, 2018030020
; Device  : ATmega16 (WARNING: If another device is used, some registers may differ a bit)
; IDE used: Microchip Studio 7
; CPU freq: 10 MHz (If changed, things such as timer init. value and UBRR for the seial port should change too)
*/

#include <avr/io.h>
#include <avr/interrupt.h>  //To use interrupt handlers
#include <avr/iom16.h>      //Definition for UDR
#include <avr/pgmspace.h>   //To be able to use the flash memory

#define F_CPU 10000000             // CPU Clock = 10MHz
#define BAUD 9600                  // Data rate of 9600 BAUD for USART
#define MYUBRR ((F_CPU/16/BAUD)-1) // Calculate UBRR's value

/************************************************************************/
/* Store the decoding data to flash                                     */
/* Based on the application note "Atmel AVR4027: Tips and Tricks to     */
/*   Optimize Your C Code for 8-bit AVR Microcontrollers"               */
/* HOWEVER, const is needed by avr-gcc in order to compile successful,  */
/*   although the app. note does not use it                             */
/************************************************************************/
const uint8_t dec_data[12] PROGMEM = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF, 0x00};

/************************************************************************/
/* Global variables                                                     */
/************************************************************************/
volatile uint8_t flag_reg;  /* -flag_reg bit0 shows if we have received A
                               -flag_reg bit1 shows if we have received T
                               -flag_reg bit2 shows if we have received C
                               -flag_reg bit3 shows if we have received N
                               -flag_reg upper nibble is useless */
volatile uint8_t num_cnt;   /* A counter for the numbers we have received, 
                                 in the case of a N<X><CR><LF> command */
//volatile uint8_t addr_show; /* Remembers the SRAM address of the digit to
//                                 show (r17 was used in Lab 4) */

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
	UDR = data; 
	
}

/************************************************************************/
/* send_ok (C equivalent of send_ok in Lab3, but here a function        */
/*             is used to transmit)                                     */
/* Function to send an OK<CR><LF> message                               */
/************************************************************************/
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
		/* If "C", flag, clear screen and return */
		flag_reg = flag_reg | 0x04;
		clear_screen();
	}
	else if (received_char == 0x4E){   //N
		/* If "N", flag, clear screen and return */
		flag_reg = flag_reg | 0x08;
		clear_screen();
	}
	else if (received_char == 0x0D){   //<CR>
		/* If "<CR>", simply return, no flag is needed */
	}
	else if (received_char == 0x0A){   //<LF>
		/* If "<LF>", process and return
		   <LF> is the last character of each command, so it 
		       is time to execute the received command 
	     */
		
		if(flag_reg == 0x03){
			/* If command was AT, send OK */
			send_ok();
		}
		else if(flag_reg == 0x04){
			/* If command was C, send OK */
			send_ok();
		}
		else if(flag_reg == 0x08){
			/* If command was N, send OK (the numbers are already saved) */
			num_cnt = 0;
			send_ok();
		}
		
	}
	else{ //Received number
		
		/* Increment the received numbers counter by 1 */
		num_cnt++;
		
		/* Convert from ASCII to BCD, using the appropriate mask */
		received_char = received_char & 0x0F;
		
		/* Shift all the stored number one position in SRAM (addresses digits[0]...digits[7])
		   This should not be done for the first number we receive */
		if(num_cnt != 1){
			//shift_all_by_one();
		}
		
		/* Write the number to the lowest address of the digits table*/
		//digits[0] = received_char; 
		
	}
}



/************************************************************************/
/* init                                                                 */
/* Function to initialize global variables, ports, counters and USART   */
/************************************************************************/
void init(void){
	
	/* Initialize the global variable for the screen (will be used by Timer 0 ISR) */
	//addr_show = 0;
	
	/* Initialize the global variables for the serial port */
	flag_reg = 0x00;
	num_cnt = 0;
	
	/* Initialize the Ports A and C */
	portA_init();
	
	/* Initialize (clear) the screen (the data of the array "digits") */
	//clear_screen();
	
	/* Initialize Timer/Counter 0 */
	//timer0_init();
	
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
	while (1) { }
	
}




