///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define F_CPU 9600000UL																			//
#include <avr/io.h>																				//
#include <avr/interrupt.h>																		//
#include <avr/pgmspace.h>																		//
																								//
//#define	UART_SPEED	    9600																		//
//#define	UART_SPEED	  19200																		//
//#define	UART_SPEED	  38400																		//
#define	UART_SPEED	  57600																		//
//#define	UART_SPEED	115200																		//
//#define	UART_SPEED	230400	 																	//
//#define	UART_SPEED	250000	 																	//
																								//
#if UART_SPEED == 9600																			//
		#define UART_DELAY	(uint8_t)246		// Delay for UART on 9600 baud						//
#endif												//	(Successfully tested in hardware (Attiny13a DIP8))	//
#if UART_SPEED == 19200																			//
		#define UART_DELAY	(uint8_t) 122		// Delay for UART on 19200 baud						//
#endif												//	(Successfully tested in hardware (Attiny13a DIP8))	//
#if UART_SPEED == 38400																			//
		#define UART_DELAY	(uint8_t) 60		// Delay for UART on 38400 baud						//
#endif												//	(Successfully tested in hardware (Attiny13a DIP8))	//
#if UART_SPEED == 57600																			//
		#define UART_DELAY	(uint8_t) 41		// Delay for UART on 57600 baud						//
#endif												//	(Successfully tested in hardware (Attiny13a DIP8))	//
#if UART_SPEED == 115200																		//
		#define UART_DELAY	(uint8_t) 19		// Delay for UART on 115200 baud						//
#endif												//	(Successfully tested in hardware (Attiny13a DIP8))	//
#if UART_SPEED == 230400																		//
		#define UART_DELAY	(uint8_t) 8			// Delay for UART on 230400 baud						//
#endif												//	(Successfully tested in hardware (Attiny13a DIP8))	//
#if UART_SPEED == 250000																		//
		#define UART_DELAY	(uint8_t) 7			// Delay for UART on 250000 baud						//
#endif												//	(Successfully tested in hardware (Attiny13a DIP8))	//
																								//
const char PGM_String0[] PROGMEM =	{ "Micro UART library for AVR" };//	Welcome message ( 27 bytes )	//
const char PGM_String1[] PROGMEM =	{ "  Running  on  ATtiny13A  " };//	    demo		    ( 27 bytes )	//
const char PGM_String2[] PROGMEM =	{ "	   on 250000 baud rate   " };//	    strings		    ( 27 bytes )	//
																								//
const	uint8_t	UART_TX_PORT	=	_SFR_IO_ADDR(PORTB);	// UART TX data port 					//
const	uint8_t	UART_RX_PORT	=	_SFR_IO_ADDR(PINB);		// UART RX data port 					//
const	uint8_t	RxD				=	PB3;					// Receiver data line						//
const	uint8_t	TxD				=	PB4;					// Transmitter data line					//
																								//
#define	 Buf_Size (uint8_t) 13								// Buffer	Size for receiving				//
char	 Rx_Buf[Buf_Size] = {'T','e','s','t','0','1','2','3','4','5',10,13,0}; // Buffer for receiving	//
volatile uint8_t	Rx_flags;					// [R0000000] Received flags register (highest bit is STOP-flag)	//
																								//
#define	Send_BEEP	UART_Send((char [2]){7,0})				// Beep Terminal macro (if Terminal support it)	//
#define	Send_CLS	UART_Send((char [2]){12,0})				// Clear Terminal macro					//
#define	Send_CR		UART_Send((char [3]){10,13,0})			// Next Terminal line macro				//
																								//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////		UART  Transmitting		/////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UART_Send(char* text){//								//						( 54 bytes )	//
asm volatile(	"	cli								\n\t"	//	Prevent Global Interrupts				//
"TX_Byte:"		"	ld		r18,		Z+			\n\t"	//	Load data byte for sending				//
				"	cp		r18,		r1			\n\t"	//	Compare with ZERO					//
				"	breq	Exit_Transmit			\n\t"	//	Exit if equal, EXIT transmitting			//
				"	dec		r1						\n\t"	//	Setup bits counter mask 				//
				"	cbi		%[port],	%[TX_line]	\n\t"	//	Setup START bit						//
"Delay_TX:"		"	mov		r0,			%[delay]	\n\t"	//	Delay loop, forming bit delay on Tx line	//
"Do_Delay_TX:"	"	nop								\n\t"	//								 |	//
				"	dec		r0						\n\t"	//								 |	//
				"	brne	Do_Delay_TX				\n\t"	//	________________________________________|	//
"TX_Bit:"		"	sbrc	r18,		0			\n\t"   //	Set current bit (0) on Tx line			//
				"	sbi		%[port],	%[TX_line]	\n\t"	//						   |			//
				"	sbrs	r18,		0			\n\t"   //						   |			//
				"	cbi		%[port],	%[TX_line]	\n\t"	//	______________________________|			//
				"	lsr		r18						\n\t"	//	Setup next bit for sending				//
				"	lsr		r1						\n\t"	//	Shift bits counter mask				//
				"	brcs	Delay_TX				\n\t"	//	If carry is NOT ZERO, continue sending	//
				"	sbi		%[port],	%[TX_line]	\n\t"	//	Set STOP bit on Tx line				//
				"	mov		r0,			%[delay]	\n\t"	//	Delay forming STOP bit on Tx line		//
"Stop_Bit_TX:"	"	nop								\n\t"	//							  |		//
				"	dec		r0						\n\t"	//							  |		//
				"	brne	Stop_Bit_TX				\n\t"	//	___________________________________|		//
				"	rjmp	TX_Byte					\n\t"	//	Jump to next sending byte 				//
"Exit_Transmit:""	sei								\n\t"	//	Allow Global Interrupts				//
:: [Buf]"z"(text), [TX_line]"I"(TxD), [delay]"r"(UART_DELAY), [port]"I"(UART_TX_PORT):"r18");	//
}																								//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////		UART  Detect Transmitting Interrupt	///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR(PCINT0_vect, ISR_NAKED) {								//	PCINT [0:5]		( 114-120 bytes )	//
asm volatile(	"	push	r0						\n\t"	//									//
				"	push	r1						\n\t"	//									//
				"	in		r0,			0x3F		\n\t"	//	Store SREG						//
				"	push	r0						\n\t"	//									//
				"	push	r18						\n\t"	//	Store used registers					//
				"	push	r26						\n\t"	//	Store X							//
				"	push	r30						\n\t"	//	Store Z							//
				"	clr		r1						\n\t"	//	Bits counter						//
:::);														//									//
asm volatile(	"	add		%[end],		r30			\n\t"	//	Set End-address of buffer				//
"Rx_Byte:"		"	sbic	%[port],	%[Rx_line]	\n\t"	//	START bit detector					//
				"	rjmp	Exit_Receive			\n\t"	//	EXIT receiver if START-bit is not detected	//
				"	dec		r1						\n\t"	//	Set Bits counter	mask to 0b11111111		//
"Delay_Rx:"		"	mov		r0,			%[delay]	\n\t"	//	Loop	 for skipping current bit			//
"Do_Delay_Rx:"	"	nop								\n\t"	//						   |			//
				"	dec		r0						\n\t"	//						   |			//
				"	brne	Do_Delay_Rx				\n\t"	//	______________________________|			//
#if UART_SPEED>115200										//	Correction		 					//
				"	nop								\n\t"	//			for UART 					//
				"	nop								\n\t"	//				speeds above 			//
				"	nop								\n\t"	//						115200		//
#endif														//									//
				"	lsr		r18						\n\t"	//	Bit loader			____________________	//
				"	sbis	%[port],	%[Rx_line]	\n\t"   //	If RX-line up					    |	//
				"	nop								\n\t"	//		... do delay					    |	//
				"	sbic	%[port],	%[Rx_line]	\n\t"   //	else							    |	//
				"	ori		r18,		0x80		\n\t"	//		... set significant data bit		    |	//
				"	lsr		r1						\n\t"	//	Shift Bits-counter mask			    |	//
				"	brne	Delay_Rx				\n\t"	//	Repeat if Bits-counter mask is not zero__|	//
				"	st		Z+,			r18			\n\t"	//	Store current byte					//
				"	cp		r30,		%[end]		\n\t"	//	Overflow buffer control				//
				"	breq	End_Receive				\n\t"	//	Exit if reached end					//
				"	mov		r0,			%[delay]	\n\t"	//	Loop for skipping last bit	________		//
"Skip:"			"	nop								\n\t"	//							  |		//
				"	nop								\n\t"	//							  |		//
				"	dec		r0						\n\t"	//							  |		//
				"	brne	Skip					\n\t"	//	___________________________________|		//
				"	dec		r0						\n\t"	//	r0 = 255							//
"Stop_Rx:"		"	nop								\n\t"	//	Loop	 for skipping STOP-bit_______		//
				"	dec		r0						\n\t"	//						 	  |		//
				"	breq	End_Receive				\n\t"	//	___________________________________|		//
				"	sbis	%[port],	%[Rx_line]	\n\t"   //	... with finding new START bit	  |		//
				"	rjmp	Rx_Byte					\n\t"	//							  |		//
				"	rjmp	Stop_Rx					\n\t"	//	___________________________________|		//
"End_Receive:"	"	ldi		r18,		0x80		\n\t"	//	Rx_flags	=	0x80					//
				"	st		X,			r18			\n\t"	//	Store Rx_flags						//
				"	st		Z,			r0			\n\t"	//	Zero string marker					//
"Exit_Receive:"	"	ldi		r18,		0x20		\n\t"	//	GIFR		=	( 1<<PCIF )			//
				"	out		0x3A,		r18			\n\t"	//	(Reset all collected PCINT0 interrupts)	//
::[flag] "x" (&Rx_flags), [Buf]"z"(Rx_Buf), \
[Rx_line]"I"(RxD), [delay]"r"(UART_DELAY), [end]"r"(Buf_Size), [port]"I" (UART_RX_PORT):"r18");	//
asm volatile(	"	pop		r30						\n\t"	//	Restore Z							//
				"	pop		r26						\n\t"	//	Restore X							//
				"	pop		r18						\n\t"	//	Restore r18						//
				"	pop		r0						\n\t"	//	Restore SREG						//
				"	out		0x3F,		r0			\n\t"	//	Restore old SREG state				//
				"	pop		r1						\n\t"	//	Restore r1							//
				"	pop		r0						\n\t"	//	Restore r0							//
				"	reti							\n\t"	//	Exit Interrupt						//
:::);														//									//
}															//									//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////		UART  Send PGM String		///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UART_Send_PGM(char* PGM_string)						//						( 76 bytes )	//
{															//									//
	uint8_t	s=0xFF, i=0;									//	Function read part ,					//
	while(s)	{											//			of PROGMEM space string		//
		while((s=pgm_read_byte(PGM_string++))&&(i<(Buf_Size-1)))//								//
			Rx_Buf[i++]=s;									//	fill the buffer,						//
		if (!s) Rx_Buf[i]=0;								//			and send buffer over UART,	//
		UART_Send(Rx_Buf);									//	then read next part					//
		if (s) { i=0; Rx_Buf[i++]=s; }						//			until reach ZERO code		//
	}														//									//
}															//									//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////		Convert BYTE to C-String		/////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* b_2_a(uint8_t num)									//						( 52 bytes )	//
{															//									//
char* Pos;													//									//
asm volatile (												//	Function use buffer					//
				"	ldi     r25,		-1 + '0'	\n"		//		not less than 4 bytes!				//
"1:"			"	inc     r25						\n"		//									//
				"	sbci    r24,		100			\n"		//									//
				"	brcc    1b						\n"		//									//
				"	st		Z+,			r25			\n"		//									//
				"	ldi     r25,		10 + '0'	\n"		//	 ;-10								//
"2:"			"	dec     r25						\n"		//									//
				"	subi    r24,		-10			\n"		//	;+10								//
				"	brcs    2b						\n"		//									//
				"	st		Z+,			r25			\n"		//									//
				"	subi    r24,		-'0'		\n"		//									//
				"	st		Z+,			r24			\n"		//									//
				"	st		Z,			r1			\n"		//									//
				"	subi    r30,		3			\n"		//									//
				"	ldi     r24,		4			\n"		//									//
"3:"			"	dec		r24						\n"		//									//
				"	breq    4f						\n"		//									//
				"	ld		r23,		Z+			\n"		//									//
				"	cpi		r23,		'0'			\n"		//									//
				"	breq    3b						\n"		//									//
"4:"			"	dec		r30						\n"		//									//
	:"=z" (Pos): "z" (Rx_Buf), [num] "r" (num):);			//									//
	return Pos;												//									//
}															//									//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////		Convert WORD to C-String		///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* w_2_a(uint16_t data)									//						( 78 bytes )	//
{															//									//
char* Pos;													//									//
asm volatile (												//	Function use buffer					//
				"	ldi     r23,		-1 + '0'	\n"		//		not less than 6 bytes!				//
"1:"			"	inc     r23						\n"		//									//
				"	subi    %A[dat],	lo8(10000)	\n"		//	;-10000							//
				"	sbci    %B[dat],	hi8(10000)	\n"		//									//
				"	brcc    1b						\n"		//									//
				"	st		Z+,			r23			\n"		//									//
				"	ldi     r23,		10 + '0'	\n"		//									//
"2:"			"	dec     r23						\n"		//									//
				"	subi    r24,		lo8(-1000)  \n"		//	;+1000							//
				"	sbci    r25,		hi8(-1000)	\n"		//									//
				"	brcs    2b						\n"		//									//
				"	st		Z+,			r23			\n"		//									//
				"	ldi     r23,		-1 + '0'	\n"		//									//
"3:"			"	inc     r23						\n"		//									//
				"	subi    r24,		lo8(100)    \n"		//	;-100								//
				"	sbci    r25,		hi8(100)	\n"		//									//
				"	brcc    3b						\n"		//									//
				"	st		Z+,			r23			\n"		//									//
				"	ldi     r25,		10 + '0'	\n"		//	 ;-10								//
"4:"			"	dec     r25						\n"		//									//
				"	subi    r24,		-10			\n"		//	;+10								//
				"	brcs    4b						\n"		//									//
				"	st		Z+,			r25			\n"		//									//
				"	subi    r24,		-'0'		\n"		//									//
				"	st		Z+,			r24			\n"		//									//
				"	st		Z,			r1			\n"		//									//
				"	subi    r30,		5			\n"		//									//
				"	ldi     r24,		6			\n"		//									//
"5:"			"	dec		r24						\n"		//									//
				"	breq    6f						\n"		//									//
				"	ld		r23,		Z+			\n"		//									//
				"	cpi		r23,		'0'			\n"		//									//
				"	breq    5b						\n"		//									//
"6:"			"	dec		 r30					\n"		//									//
	:"=z" (Pos): "z" (Rx_Buf), [dat] "r" (data):);			//									//
	return Pos;												//									//
}															//									//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////		Convert number to hex representation		///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char*  b_2_hex(uint8_t num)									//						( 46 bytes )	//
{															//									//
	uint8_t	dh = num >> 4;									//	Function use buffer					//
	Rx_Buf[0] = (dh < 10) ? (dh + '0'): (dh + 'A' - 10); 	//		not less than 3 bytes!				//
	dh = num & 0x0F;										//									//
	Rx_Buf[1] = (dh < 10) ? (dh + '0'): (dh + 'A' - 10); 	//									//
	Rx_Buf[2] = 0;											//									//
	return Rx_Buf;											//									//
}															//									//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////		Convert number to binary representation		///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* b_2_bin(uint8_t num)									//						( 32 bytes )	//
{															//									//
	for (uint8_t i=0;i<8;i++){								//	Function use buffer					//
		Rx_Buf[7-i] = (num & 1)+'0';						//		not less than 9 bytes!				//
		num>>=1;											//									//
	}														//									//
	Rx_Buf[8] = 0;											//									//
	return Rx_Buf;											//									//
}															//									//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////		UART  Receiver Init		/////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Init_UART_Receiving()									//						( 12 bytes )	//
{															//									//
	GIMSK		= (1<<PCIE);								//	Enable External Interrupts				//
	PCMSK		= (1<<RxD);									//	Enable accorded Interrupt (PCINT3)		//
	sei();													//	Allow Interrupts						//
}															//									//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void)												//						( 232 bytes )	//
{															//									//
	DDRB	 =	( 0 << RxD ) | ( 1 << TxD );				//	Port Configuration					//
	PORTB	|=	( 1 << TxD );								//	UP Transmitting line					//
	Init_UART_Receiving();									//	Setting Receiver Interrupt				//
	Send_BEEP;												//									//
	uint16_t counter=0;										//									//
	uint16_t c2=0;											//									//
	UART_Send_PGM( ( char* ) PGM_String0 );	Send_CR;		//									//
	UART_Send_PGM( ( char* ) PGM_String1 );	Send_CR;		//									//
	UART_Send_PGM( ( char* ) PGM_String2 );	Send_CR;		//									//
	Send_CR;												//									//
	while(1){												//									//
		if ( Rx_flags & 0x80 ) {							//	Test receiving flag					//
//			Send_CLS;											//	Send clear terminal screen code			//
//			Send_CR;											//	Send new terminal string				//
			UART_Send("Receive: ");							//	Send description					//
			UART_Send(Rx_Buf);								//	Send received	buffer				//
			Rx_flags=0;										//	Clear receiving flag					//
		}													//									//
		if ( !(++counter % 4000)) {							//	Simple timer						//
			UART_Send(b_2_bin(c2));	UART_Send(" ");			//	Send byte in binary representation		//
			UART_Send(b_2_hex(c2));	UART_Send(" ");			//	Send byte in hex representation			//
			UART_Send(b_2_a(c2));	UART_Send(" ");			//	Send byte in decimal representation		//
			UART_Send(w_2_a(c2++));							//	Send word in decimal representation		//
			Send_CR;										//									//
			counter=0;										//									//
			}												//									//
//		if (++counter==0) c2++;									//	Simple ping timer					//
//		if ((c2==15)&&(counter==0))	 {UART_Send(".");c2=0;}			//									//
	}														///////////////////////////////////////////////////////
}//															//	        (c) by Dolphin_Soft #101546015		//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////.