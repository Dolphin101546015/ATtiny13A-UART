# ATtiny13A-UART
Asynchron software UART for ATtiny 13A (or any AVR microCPU)

Writed in Atmel Studio 7.0 (C + Inline Assembler)

Features:
  + No timer interrupt used for Sending/Receiving;
  + Realy tiny code size (54 bytes for sending fucntion, 114 bytes for receiver interrupt);
  + Receiver with automatic synchronization, synchronizing by STOP-bit of the flow transmitting;
  + No nested function calls for Sending/Receiving;
  + No memory used for variables, except receiving buffer (13 bytes by default), and flag variable;
  + Applied receiving buffer overflow detection;
  + Applied lost line detection.
  + Flexible configuration for UART port pins;
  + Flexible UART speed rate selection;
  + Applied PGM-string send function;
  + Applied binary representation convertion function;
  + Applied hex representation convertion function;
  + Applied byte to string convertion function;
  + Applied word to string convertion function;
  + Applied speeds from standart UART protocol range 9600-250000;

Disadvantages:
  - Only zero-endings C-String sending data is possible (to get rid of nested function calls).
