/*
; main_sim.c
; 
; Copy of main.c, made for simulation purposes
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
volatile uint8_t digits[8]; /* Keep the digits to show in screen */
volatile uint8_t flag_reg;  /* -flag_reg bit0 shows if we have received A
                               -flag_reg bit1 shows if we have received T
                               -flag_reg bit2 shows if we have received C
                               -flag_reg bit3 shows if we have received N
                               -flag_reg upper nibble is useless */
volatile uint8_t num_cnt;   /* A counter for the numbers we have received, 
                                 in the case of a N<X><CR><LF> command */
volatile uint8_t addr_show; /* Remembers the SRAM address of the digit to
                                 show (r17 was used in Lab 4) */

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
/* Function to initialize Ports A and C (to interface the screen)       */
/************************************************************************/
void ports_init (void){
	
	/* PortA - which segments will light */
	DDRA = 0xFF;   /* PortA will be output */
	PORTA = 0xFF;  /* Turn all the segments off */
	
	/* PortC - which of the 8 digits will be enabled (a ring counter) */
	DDRC = 0xFF;   /* PortC will be output */
	PORTC = 0x80;  /* Initialize with 0b1000_0000 */
	
	/*PortD - serial port
	 RxD is Port D, Bit 0
	 TxD is Port D, Bit 1

	 We do nothing, because regardless of the values of the Data Direction Register
	   for Port D, if USART transmitter is enabled, RxD is configured as input and
	   TxD is configured as output automatically (pg 64/357 of the manual)
	*/
}

/************************************************************************/
/* timer0_init  (C equivalent of timer0_init in Lab3)                   */
/* Function to initialize timer0                                        */
/************************************************************************/
void timer0_init (void){
	
	/* Set the prescaler (CK/256) */
	TCCR0 = (1 << CS02) | (0 << CS01) | (0 << CS00); 
	
	/* TIMER/COUNTER 0 goes only up. We want it to count 163 numbers, so we initialize with 
	       the value 256-x=163=> x= 256-163=93 */
	TCNT0 = 93;
	
	/* Enable the TIMER0 ovf interrupt (TOIE0 = 1) */
	TIMSK = (1 << TOIE0);
}

/************************************************************************/
/* clear_screen (C equivalent of clear_screen in Lab3, but here it is   */
/*                 implemented using a loop)                            */
/* Function to clear the screen (by writing specific data to SRAM)      */
/************************************************************************/
void clear_screen (void){
	for(int i=0; i < 8; i++){
		digits[i] = 0x0A;
	}
}


/************************************************************************/
/* shift_all_by_one (C equivalent of shift_all_by_one in Lab3)          */
/* Function to shift the data in SRAM by one position                   */
/************************************************************************/
void shift_all_by_one (void){
	/* Two temp variables are needed*/
	uint8_t tmp1;
	uint8_t tmp2;
	
	/* new_digit0= 0xA0 */
	tmp1 = digits[0];
	digits[0] = 0x0A;
	
	/* new_digit1 = old_digit0 */
	tmp2 = digits[1];
	digits[1] = tmp1;
	
	/* new_digit2 = old_digit1 */
	tmp1 = digits[2];
	digits[2] = tmp2;
	
	/* new_digit3 = old_digit2 */
	tmp2 = digits[3];
	digits[3] = tmp1;
	
	/* new_digit4 = old_digit3 */
	tmp1 = digits[4];
	digits[4] = tmp2;
	
	/* new_digit5 = old_digit4 */
	tmp2 = digits[5];
	digits[5] = tmp1;
	
	/* new_digit6 = old_digit5 */
	tmp1 = digits[6];
	digits[6] = tmp2;
	
	/* new_digit7 = old_digit6 */
	digits[7] = tmp1;	
	
	/* old_digit7 is thrown away */
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
	TCNT1L = data; /* instead of "UDR = data;" for testing */
	
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
	
	/* Added for simulation */
	received_char = UDR;  /* Read UDR one more time, to consume RXC flag */
	received_char = TCNT2;
	
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
			shift_all_by_one();
		}
		
		/* Write the number to the lowest address of the digits table*/
		digits[0] = received_char; 
		
	}
}

/************************************************************************/
/* The interrupt service routine for the Timer0 overflow event          */
/* (C equivalent of TIMER0_OVF_vect in Lab4)                            */
/* Changes from Lab 4:                                                  */
/*  ~addr_show now stores the index in the array, and not a memory      */
/*       address, it takes values 0-7                                   */
/*  ~The code is much more condensed, because C is high-level, unlike   */
/*     assembly, where everything was done manually                     */
/************************************************************************/
ISR (TIMER0_OVF_vect){
	/* Turn off all the segments */
	PORTA = 0xFF;
	
	/* Go to the right digit */
	if (PORTC == 0x80){ /* If PortC = 0x80, reset counters/addresses (go to Least Sign. Digit) */
		addr_show = 0;
		PORTC = 0x01;
	}
	else{ /* Simply continue to the next digit */ 
		addr_show++;
		PORTC = PORTC << 1;
	}
	
	/* Load from SRAM the digit that will be shown (in BCD format) in a temporary local variable */
	uint8_t digit_in_bcd = digits[addr_show];
	
	/* Mask to get rid of the upper nibble */
	digit_in_bcd = digit_in_bcd & 0x0F;
	
	/* Send the 7seg representation to PORTA */
	PORTA = pgm_read_byte(&dec_data[digit_in_bcd]);
	
	/* Reset the counter (to achieve a frequency of 240Hz) */
	TCNT0 = 93;
}

/************************************************************************/
/* init                                                                 */
/* Function to initialize global variables, ports, counters and USART   */
/************************************************************************/
void init(void){
	
	/* Initialize the global variable for the screen (will be used by Timer 0 ISR) */
	addr_show = 0;
	
	/* Initialize the global variables for the serial port */
	flag_reg = 0x00;
	num_cnt = 0;
	
	/* Initialize the Ports A and C */
	ports_init();
	
	/* Initialize (clear) the screen (the data of the array "digits") */
	clear_screen();
	
	/* Initialize Timer/Counter 0 */
	timer0_init();
	
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



