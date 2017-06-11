# ATtiny13A-UART
Asynchron software UART for ATtiny 13A (or any AVR microCPU)

Writed in Atmel Studio 7.0 (C + Inline Assembler)

Features:
  + No timer interrupt used for Sending/Receiving;
  + Realy tiny code size (56 bytes for sending fucntion, 80 bytes for receiver interrupt);
  + Receiver with automatic synchronization, synchronizing by STOP bit of the flow transmitting;
  + No nested function calls for Sending/Receiving;
  + No memory used for variables, except receiving buffer (13 bytes by default);
  + Applied receiving buffer overflow detection;
  + Applied lost line detection.
  + Flexible configuration for UART port pins;
  + Flexible UART speed rate selection;

Disadvantages:
  - Still not tested in hardware;
  - Not tested (and still not configured) for speed rates above 57600 baud;
  - Only zero-endings C-String sending data is possible (to get rid of nested function calls).
