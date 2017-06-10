# ATtiny13A-UART
Asynchron software UART for ATtiny 13A (or any AVR microCPU)

Features:
  + No timer interrupt used for Sending/Receiving;
  + Realy tiny code size (56 bytes for sending fucntion, 74 bytes for receiver interrupt);
  + Receiver with automatic synchronization, synchronizing by STOP bit of the flow transmitting;
  + No nested function calls for Sending/Receiving;
  + Flexible configuration for UART port pins;
  + No memory used for variables, except receiving buffer (13 bytes by default);
  + Only three registers used for both UART functions, except temporary (r0);
  + Flexible UART speed configuration;

Disadvantages:
  - Still not tested in hardware;
  - Not tested (and still not configured) for speeds above 57600 baud;
  - Only zero-endings C-String sending data is possible (to get rid of nested function calls)
  - Receiving buffer overflow control is not implemented (for reducing code size);
