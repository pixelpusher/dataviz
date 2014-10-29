/*
 * EEPROM Write/ READ
 *
 * Stores values read from analog input 0 into the EEPROM.
 * These values will stay in the EEPROM when the board is
 * turned off and may be retrieved later by another sketch.
 */

#include <EEPROM.h>

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
uint16_t addr = 0;
uint16_t ADDRESS_HEADER_LENGTH = 2; // in bytes

uint16_t NUM_ADDRESSES = 1024-ADDRESS_HEADER_LENGTH;


void readLastAddress()
{
  
  // read a byte from the current address of the EEPROM
  uint16_t addrLoValue = EEPROM.read(0);
  uint16_t addrHiValue = EEPROM.read(1);

  addr |= addrLoValue; // bottom 8 bits  
  addr |= (addrHiValue << 8); // top 8 bits

  Serial.print("last address:");
  Serial.println(addr+ADDRESS_HEADER_LENGTH);
  
}

uint16_t writeLastAddress()
{  
  byte lo = (addr & 255); // 0 out top 8 bits
  byte hi = (addr >> 8) & 255; //shift top bits right by 8 
  
    // write address
  EEPROM.write(0,lo );
  EEPROM.write(1, hi );
}



void setup()
{
  Serial.begin(57600);
  readLastAddress();
  delay(100); // wait a short bit...
}

void loop()
{
  // advance to the next address.  there are 512 bytes in 
  // the EEPROM, so go back to 0 when we hit 512.
  addr = (addr + 1) % NUM_ADDRESSES; // account for top 2 byte of address space
  
  // need to divide by 4 because analog inputs range from
  // 0 to 1023 and each byte of the EEPROM can only hold a
  // value from 0 to 255.
  int val = analogRead(0) / 4;
  
  // write the value to the appropriate byte of the EEPROM.
  // these values will remain there when the board is
  // turned off.
  EEPROM.write(addr+ADDRESS_HEADER_LENGTH, val);

  writeLastAddress();
 
  Serial.print((addr+ADDRESS_HEADER_LENGTH));
  Serial.print(": ");
  Serial.println(val);
    
  
  delay(1500);
}
