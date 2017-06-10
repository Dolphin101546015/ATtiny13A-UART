////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define F_CPU 9600000UL																																															//
#include <avr/io.h>																																																		//
#include <avr/interrupt.h>																																															//
																																																										//
//#define UART_SPEED_19200																																													//
#define UART_SPEED_57600																																														//
																																																										//
#ifdef UART_SPEED_9600																																															//
		#define UART_DELAY	238										//   9600 bod																													//
#endif																																																								//
																																																										//
#ifdef UART_SPEED_19200																																															//
		#define UART_DELAY	119										// 19200 bod																														//
#endif																																																								//
#ifdef UART_SPEED_38400																																															//
		#define UART_DELAY	59										// 38400 bod																														//
#endif																																																								//
																																																										//
#ifdef UART_SPEED_57600																																															//
		#define UART_DELAY	40										// 57600 bod																														//
#endif																																																								//
																																																										//
//const uint8_t		Rx_Delay	=	123;								// 9600																																//
//const		uint8_t		Rx_Delay	=	58;						// 19200																																//
																																																										//
const		uint8_t			UART_OUT_PORT	=	_SFR_IO_ADDR(PORTB);							//	UART TX data port 													//
const		uint8_t			UART_IN_PORT		=	_SFR_IO_ADDR(PINB);								//	UART RX data port 													//
const		uint8_t			RxD	=	PB3;								//	Receiver data line																										//
const		uint8_t			TxD	=	PB4;								//	Transmitter data line																									//
																																																										//
#define	Buffer_Size 13													//	Buffer	Size for receiving																								//
char											Rx_Buf[Buffer_Size]	=	{'T','e','s','t',' ','0','1','2','3',10,13,0};	//	 Buffer for receiving											//
volatile	char*						Rx_Ptr		=	&Rx_Buf[0];			//	Current receiving byte pointer																			//
volatile	register	uint8_t	Rx_flags	asm("r16");				//	[R0000000] Received flags register (highest bit is STOP-flag)					//																								//
																																																										//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////		UART  Transmitting			////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void UART_Send(char *text){																									//																					//
	asm volatile(																															//																					//
							"	cli															\n\t"												//		Deny Interrupts												//
"Tx_Byte:	"		"	ld			r18,			X+							\n\t"												//		Load data byte for sending							//
							"	cp		r18,			r1								\n\t"												//		Compare with ZERO										//
							"	breq	Exit_Transmit							\n\t"												//		Exit if equal, EXIT transmitting						//
							"	ldi		r19,			9								\n\t"												//		Setup bits counter											//
							"	cbi		%[port],	%[Tx_line]				\n\t"												//		Setup START bit												//
"Delay_Tx:"		"	mov		r0	,			%[delay]					\n\t"												//		Delay loop, forming bit delay on Tx line		//
"Do_Delay_Tx:""	nop														\n\t"												//																				|	//
							"	dec		r0												\n\t"												//																				|	//
							"	brne	Do_Delay_Tx							\n\t"												//		_________________________________________|	//
"Tx_Bit:	"			"	sbrc		r18,			0								\n\t"   												//		Set current bit (0) on Tx line							//
							"	sbi		%[port],	%[Tx_line]				\n\t"												//																		|			//
							"	sbrs		r18,			0								\n\t"   												//																		|			//
							"	cbi		%[port],	%[Tx_line]				\n\t"												//		____________________________________|			//
							"	lsr		r18											\n\t"												//		Setup next bit for sending								//
							"	dec		r19											\n\t"												//		Decreasing bits counter									//
							"	brne	Delay_Tx									\n\t"												//		If not ZERO, continue sending						//
							"	sbi		%[port],	%[Tx_line]				\n\t"												//		Set STOP bit on Tx line									//
							"	mov		r0,			%[delay]					\n\t"												//		Delay forming STOP bit on Tx line				//
"Stop_Bit_Tx:"	"	nop														\n\t"												//																				|	//
							"	dec		r0												\n\t"												//																				|	//
							"	brne	Stop_Bit_Tx							\n\t"												//		_________________________________________|	//
							"	rjmp	Tx_Byte									\n\t"												//		Jump to next sending byte 							//
"Exit_Transmit:""	sei														\n\t"												//		Allow Interrupts												//
	:: [Buf] "x" (text), [Tx_line]	"I" (TxD), [delay]	"r" (UART_DELAY), [port]	"I" (UART_OUT_PORT):"r18","r19");											//
}																																																										//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////		UART  Detect Transmitting Interrupt	///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__attribute__ ((naked)) ISR(PCINT0_vect) {																			// PCINT [0:5]															//
//	if (!(PINB&(1<<RxD))) Rx_flags	=	0x80;																		//																					//
	asm volatile(																															//																					//
							"	cli															\n\t"												//		Deny Interrupts												//
							"	push	r18											\n\t"												//		Store r18															//
							"	push	r19											\n\t"												//		Store r19															//
"Rx_Byte:"			"	sbic		%[port],	%[Rx_line]				\n\t"												//		START bit detector											//
							"	rjmp	Exit_Receive							\n\t"												//																					//
							"	ori		r16,			0x80						\n\t"												//		Rx_flags	=	0x80; (Data received flag)			//
							"	ldi		r18,			8								\n\t"												//		Bits counter														//
"Delay_Rx:"		"	mov		r0,			%[delay]					\n\t"												//		Loop	for skipping current bit						//
"Do_Delay_Rx:""	nop														\n\t"												//																|					//
							"	dec		r0												\n\t"												//																|					//
							"	brne	Do_Delay_Rx							\n\t"												//		_______________________________|						//
							"	lsr		r19											\n\t"												//		Bit loader															//
							"	sbic		%[port],	%[Rx_line]				\n\t"   												//		              		|													//
							"	ori		r19,			0x80						\n\t"												//								|													//
							"	dec		r18											\n\t"												//		_____________|														//
							"	brne	Delay_Rx								\n\t"												//																					//
							"	st			X+,			r19							\n\t"												//		Store current byte											//
							"	mov		r0,			%[delay]					\n\t"												//		Loop	for skipping last & stop bits				//
"Skip:"					"	nop														\n\t"												//																		|			//
							"	dec		r0												\n\t"												//																		|			//
							"	brne	Skip											\n\t"												//		____________________________________|			//
							"	dec		r0												\n\t"												//		r0 = 255																//
"Stop_Rx:"			"	nop														\n\t"												//		IDLE loop															//
							"	dec		r0												\n\t"												//																			|		//
							"	breq	Exit_Receive							\n\t"												//				__________________________________|		//
							"	sbic		%[port],	%[Rx_line]				\n\t"   												//		... with finding new START bit				|		//
							"	rjmp	Stop_Rx									\n\t"												//																			|		//
							"	rjmp	Rx_Byte									\n\t"												//		______________________________________|			//
"Exit_Receive:"	"	st			X,				r1								\n\t"												//		Zero string marker											//
							"	pop		r19											\n\t"												//		Restore r19														//
							"	pop		r18											\n\t"												//		Restore r18														//
							"	sei														\n\t"												//		Allow Interrupts												//
							"	reti														\n\t"												//		Return from Interrupt										//
:: [Buf] "x" (Rx_Buf), [Rx_line]	"I" (RxD), [delay]	"r" (UART_DELAY+1), [port]	"I" (UART_IN_PORT):"memory");										//
}																																					//																					//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////		UART  Receiver Init		////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Init_UART_Receiving()																																															//
{																																					//																					//
	GIMSK		= (1<<PCIE);																											//	Enable External Interrupts									//
	PCMSK	= (1<<PCINT3);																										//	Enable PCINT3 Interrupt									//
	sei();																																		//	Allow Interrupts													//
}																																																										//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*																																																										//
void UART_Send_uint16(uint16_t n){																																										//
	uint8_t i = 0;																																																				//
	for (i=0; i<5; i++) Rx_Buf[i]=32; i=5;Rx_Buf[i--]=0;																																			//
	do { Rx_Buf[i--] = n % 10 + '0'; }																																											//
	while ((n /= 10) > 0);																																																//
	UART_Send(Rx_Buf);																																																//
}	*/																																																									//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void)																																																				//
{																																																										//
	DDRB		  =	(0<<RxD) | (1 << TxD) ;																					//	Port Configuration											//
	PORTB		|=	(1 << TxD);																																															//
//	#define UART_Sender																																																//
	#define UART_Receiver																																															//
																																																										//
	#ifdef UART_Receiver																																																//
		Init_UART_Receiving();																																														//
	#endif																																																							//
																																																										//
	#ifdef UART_Sender																																																	//
		uint16_t counter=0;																																																//
	#endif																																																							//
																																																										//
//	UART_Send((char [2]){7,0});																																													//
	while(1){																																																						//
			#ifdef UART_Sender																																															//
				if ((++counter)%10000==0)	 UART_Send(Rx_Buf);																																	//
//				if ((++counter)%150000==0)	 UART_Send((char [2]){12,0});																											//
			#endif																																																					//
																																																										//
			#ifdef UART_Receiver																																														//
				if (Rx_flags&0x80) {																																														//
//							UART_Send((char [2]){12,0});																																							//
//							UART_Send_uint16(counter++);																																					//
	 						UART_Send("Receive: ");																																									//
							UART_Send(Rx_Buf);																																										//
							Rx_flags=0;																																														//
				}																																																						//
			#endif																																																					//
	}																																																									//
}																																																										//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

