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
  for(int base = 0; base <= 2047; base += 16) {
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

// 4-bit hex decoder for common cathode 7-segment display
// byte data[] = {0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47};

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(9600);

//  clearEEPROM();
//  printContents();
//
//  Serial.print("Writing EEPROM");
//  for(int address = 0; address <= sizeof(data); address++) {
//    writeEEPROM(address, data[address]);
//    Serial.print(".");
//  }
//  Serial.println("DONE.");

//  Serial.println("Programming EEPROM...");
    byte digits[] = {0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b};
//  // Write the one's places
//  for(int value = 0; value <= 255; value += 1) {
//    writeEEPROM(value, digits[value % 10]);
//  }
//  // Write the ten's places
//  for(int value = 0; value <= 255; value += 1) {
//    writeEEPROM(value + 256, digits[(value / 10) % 10]);
//  }
//  // Write the hundred's places
//  for(int value = 0; value <= 255; value += 1) {
//    writeEEPROM(value + 512, digits[(value / 100) % 10]);
//  }
//  // Write all zeros in the case where A8 and A9 are both high
//  for(int value = 0; value <= 255; value += 1) {
//    writeEEPROM(value + 768, 0);
//  }

// Negative numbers
    Serial.println("Programming negative numbers into the EEPROM...");
    // one's places
    Serial.println("Programming one's places");
    for(int value = -128; value <= 127; value += 1) {
      writeEEPROM((byte)value + 1024, digits[(abs(value) % 10)]);
    }
    // ten's places
    Serial.println("Programming ten's places...");
    for(int value = -128; value <= 127; value += 1) {
      writeEEPROM((byte)value + 1280, digits[(abs(value) / 10) % 10]);
    }
    // hundred's places
    Serial.println("Programming hundred's places...");
    for(int value = -128; value <= 127; value += 1) {
      writeEEPROM((byte)value + 1536, digits[(abs(value) / 100) % 10]);
    }
    // negative sign
    Serial.println("Programming negative sign...");
    for(int value = -128; value <= 127; value += 1) {
      if (value < 0) {
        writeEEPROM((byte)value + 1792, 0x01);
      } else {
        writeEEPROM((byte)value + 1792, 0);
      }
    }
  Serial.println("Done.");
  printContents();

}

void loop() {
   
}
