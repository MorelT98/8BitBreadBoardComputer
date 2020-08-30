#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80)); // 0x80 is 100000...000 in binary. If outputEnable is true, the last bit of the second register will be set, which we'll use for our EEPROM output enable signal
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address); 

  // toggle the storage register input
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readEEPROM(int address) {
    // Set all the data pins as inputs
    for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
      pinMode(pin, INPUT);
    }
  
  // Tell the EEPROM which address we're reading from
  setAddress(address, true);
  byte data = 0;
  // Read EEPROM pins one by one and append it to data variable
  for(int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

void printContents() {
  // Set all the data pins as inputs
    for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
      pinMode(pin, OUTPUT);
    }
  
  // Read the EEPROM 16 bytes at the time
  for(int base = 0; base <= 255; base += 16) {
    byte data[16];
    for(int offset = 0; offset <= 15; offset++) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    // %03x: 3 digit hex number with leading 0s
    // %02x: 2 digit hex number with leading 0s
    sprintf(buf, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
     base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf);
  }
}

void writeEEPROM(int address, byte data) {
  // Set all the data pins as outputs
    for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
      pinMode(pin, OUTPUT);
    }
  
  setAddress(address, false); // Set outputEnable to false, since we're now inputting data into the EEPROM
  for(int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    digitalWrite(pin, data & 1); // & 1 selects the last bit of data (the least significant)
    data = data >> 1;
  }
  // Serial.println("Sent the data to the EEPROM pins.");

  // Pulse the write_enable
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1); // The write pulse in the EEPROM has to be between 100ns and 1000ns (1us), but the best the arduino can do is 1us
  digitalWrite(WRITE_EN, HIGH);
  delay(10);  // Give EEPROM time to read all data
  // Serial.println("Toggled the writeEnable.");
}

void clearEEPROM() {
  Serial.print("Erasing EEPROM");
  for(int address = 0; address <= 2047; address++) {
    writeEEPROM(address, 0xff);

    if((address + 1) % 64 == 0) {
      Serial.print(".");
    }
  }
  Serial.println("DONE.");
}

#define HLT 0b1000000000000000
#define MI  0b0100000000000000
#define RI  0b0010000000000000
#define RO  0b0001000000000000
#define IO  0b0000100000000000
#define II  0b0000010000000000
#define AI  0b0000001000000000
#define AO  0b0000000100000000
#define EO  0b0000000010000000
#define SU  0b0000000001000000
#define BI  0b0000000000100000
#define OI  0b0000000000010000
#define CE  0b0000000000001000
#define CO  0b0000000000000100
#define J   0b0000000000000010


uint16_t data[] = {
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //0000 - NOP (Fetch)
  MI|CO, RO|II|CE,    IO|MI,    RO|AI,      0,      0, 0, 0,            //0001 - LDA
  MI|CO, RO|II|CE,    IO|MI,    RO|BI,      EO|AI,  0, 0, 0,            //0010 - ADD
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //0011 - NOP
  MI|CO, RO|II|CE,    IO|MI,    AO|RI,      0,      0, 0, 0,            //0100 - STA
  MI|CO, RO|II|CE,    IO|AI,    0,          0,      0, 0, 0,            //0101 - LDI
  MI|CO, RO|II|CE,    IO|J,     0,          0,      0, 0, 0,            //0110 - JMP
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //0111 - NOP
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //1000 - NOP
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //1001 - NOP
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //1010 - NOP
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //1011 - NOP
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //1100 - NOP
  MI|CO, RO|II|CE,    0,        0,          0,      0, 0, 0,            //1101 - NOP
  MI|CO, RO|II|CE,    AO|OI,    0,          0,      0, 0, 0,            //1110 - OUT
  MI|CO, RO|II|CE,    HLT,      0,          0,      0, 0, 0,            //1111 - HLT
};

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(9600);

  clearEEPROM();

  // Program data bytes
  Serial.print("Programming EEPROM");
  for(int address = 0; address < sizeof(data)/sizeof(data[0]); address += 1) {
    writeEEPROM(address, data[address] >> 8);  // First EEPROM
  }
  for(int address = 0; address < sizeof(data)/sizeof(data[0]); address += 1) {
    writeEEPROM(address + 128, data[address]);  // Second EEPROM
  }

  Serial.println(" Done.");
  printContents();

}

void loop() {
   
}
