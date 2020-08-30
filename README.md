# 8BitBreadBoardComputer

Starting from transistors, resistors, ICs, and other basic electronic components, I build a 8-bit computer on a breadboard! In this case, 8 bits means:

- The numbers used are at most 8-bits (0 to 255 if using positive integers only, -128 to 127 if using signed integers)
- The values stored in the RAM are 8 bits (but the addresses are 4 in length, which allows a total of 16 possible memory locations)
- The instructions excecuted by the computer are 8-bits long as well

There are two parts of this computer that needed to be programmed: The display module and the instruction decoder. To program them, I used EEPROMs (Electrically Erasable Programmable Read Only Memory), which I programmed with an Arduino Uno.

For the display module, the EEPROMs are programmed to take in a number in binary and return the corresponding decimal number on a seven segment display.
For example, if the number 00000100 , which is 4 in decimal, is passed in to the EEPROM, the return value will be 00110011, which is equivalent to 7 on the seven segment display, as follows:

  0   0   1   1   0   0   1   1
  dp  a   b   c   d   e   f   g
  
  ![4 on seven segment display]
  (https://github.com/MorelT98/8BitBreadBoardComputer/blob/master/seven-segment-4.JPG)
