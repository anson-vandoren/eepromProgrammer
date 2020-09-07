#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13
#define OUT_ENA A0  // Hook OE to A0 and treat as digital out

// data for seven-segment decoder
byte data[] = { 0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f, 0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38 };

void setAddress(int address) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  // toggle the shift latch
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readEEPROM(int address) {
  // set data pins back to inputs
  setDataOut(false);
  digitalWrite(OUT_ENA, LOW); // enable data output
  setAddress(address);
  
  
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

void setDataOut(bool dataOut) {
  if (dataOut) {
    DDRB = DDRB | B00011111;  // D8-D12
    DDRD = DDRD | B11100000;  // D5-D7
  } else {
    DDRB = DDRB & B11100000;  // D8-D12
    DDRD = DDRD & B00011111;  // D5-D7
  }
}

void writeEEPROM(int address, byte data) {
  digitalWrite(OUT_ENA, HIGH);  // disable data output
  setAddress(address);

  setDataOut(true);
  bool msb = data >> 7;  // MSB read back in after the write will give complement until write is complete
    
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  
  // toggle write signals
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);

  // wait for the write to be done
  setDataOut(false);
  digitalWrite(OUT_ENA, LOW);  // allow data output from EEPROM
  bool msb_in = !msb;          // start with complement
  while (msb_in != msb) {      // loop until D7 is uncomplemented
    msb_in = digitalRead(EEPROM_D7);
  }

}

void eraseEEPROM() {
  for (int address = 0; address <= 255; address += 1) {
    writeEEPROM(address, 0xff);
  }
}

void printContents() {
  for (int base = 0; base <= 255; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    sprintf(buf, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x    %02x %02x %02x %02x %02x %02x %02x %02x",
      base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
      data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(OUT_ENA, HIGH);
  pinMode(OUT_ENA, OUTPUT);
  
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);

  eraseEEPROM();
  
  // Bit patterns for the digits 0..9 (common cathode)
  byte digits[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x73 };
  for(int i=0; i<10; i++) {
    writeEEPROM(i, digits[i]);
  }
  printContents();

}

void loop() {
  // put your main code here, to run repeatedly:

}
