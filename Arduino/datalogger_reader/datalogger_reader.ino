/// Hooking up a DS1307 (5V) or DS1340Z (3V) real time clock via I2C.
// 2009-09-17 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

// the real-time clock is connected to port 1 in I2C mode (AIO = SCK, dIO = SDA)

// todo: hit a button (to reset the time) 

#include <JeeLib.h>
#include <EEPROMex.h>
#include <EEPROMVar.h>

#define _EEPROMEX_DEBUG //comment out for production code...


PortI2C myport (1 /*, PortI2C::KHZ400 */);
DeviceI2C rtc (myport, 0x68);



//
// time variables
//
byte now[6];
byte before[6];

const byte hourIndex = 3; // index in date array of hour byte
const byte minIndex = 4;  // etc
const byte secIndex = 5;  // etc


static byte bin2bcd (byte val) {
  return val + 6 * (val / 10);
}

static byte bcd2bin (byte val) {
  return val - 6 * (val >> 4);
}


//
// Greater-than date
// Compares left (dl) to right (dr), returns true if left is later in date than right
// assuming 24 hr clock cycles where 12am is earliest
//
boolean gtDate( byte* dl, byte* dr )
{
  // 00 11 22
  // hh mm ss

  int i=0;

  while( i < 6)
  {
    if (dl[i] > dr[i]) return true;
    if (dl[i] < dr[i]) return false;
    i++;
  }

  // if both are equal, return false
  return false;
}

const int MINS_PER_HOUR = 60;
const int SECS_PER_MIN = 60;


int diffHours( byte* dl, byte* dr )
{
  // yy mm dd h m s

  return dl[hourIndex]-dr[hourIndex];
}


int diffMins( byte* dl, byte* dr )
{
  // yy mm dd h m s

  return dl[minIndex]-dr[minIndex] + diffHours(dl, dr)*MINS_PER_HOUR;
}


int diffSecs( byte* dl, byte* dr )
{
  // yy mm dd h m s
  return dl[secIndex]-dr[secIndex] + diffMins(dl, dr)*SECS_PER_MIN;
}


static void setDate (byte yy, byte mm, byte dd, byte h, byte m, byte s) {
  rtc.send();
  rtc.write(0);
  rtc.write(bin2bcd(s));
  rtc.write(bin2bcd(m));
  rtc.write(bin2bcd(h));
  rtc.write(bin2bcd(0));
  rtc.write(bin2bcd(dd));
  rtc.write(bin2bcd(mm));
  rtc.write(bin2bcd(yy));
  rtc.write(0);
  rtc.stop();
}

static void getDate (byte* buf) {
  rtc.send();
  rtc.write(0);	
  rtc.stop();

  rtc.receive();
  buf[secIndex] = bcd2bin(rtc.read(0));
  buf[minIndex] = bcd2bin(rtc.read(0));
  buf[hourIndex] = bcd2bin(rtc.read(0));
  rtc.read(0);
  buf[2] = bcd2bin(rtc.read(0));
  buf[1] = bcd2bin(rtc.read(0));
  buf[0] = bcd2bin(rtc.read(1));
  rtc.stop();
}


void printDate(byte *d, byte length)
{
  for (byte i = 0; i < length; i++) {
    Serial.print((int) d[i]);
    Serial.print('.');
  }
  Serial.println();
}


int nextAddr;


void setup() {
  Serial.begin(57600);
  delay(100);
  Serial.println("\n[data_reader]");

    // read last address... this one should always be empty as it gets written AFTER the data is written
    nextAddr = EEPROM.readInt(0);

  Serial.print("last memory address: ");
  Serial.println(nextAddr);

  EEPROM.setMemPool(2,nextAddr); // top 2 bytes (an int) are for the nextAddr
  
  int addr = EEPROM.getAddress(sizeof(byte));
  
  byte date[3];  
  byte data;
  
  while(addr > 0 && addr <= (nextAddr-4)) // before we hit the top address...
  {
    while(!EEPROM.isReady());
    data = EEPROM.read(addr);
    Serial.print(addr);
    Serial.print(",");
    Serial.print(data);
    Serial.print(",");
    addr = EEPROM.getAddress(sizeof(byte)*3); 
    
    while(!EEPROM.isReady());
    EEPROM.readBlock<byte>(addr, date, 3);
    printDate(date,3);
/*
    Serial.print("-");
    Serial.print(date[0]);
    Serial.print(",");
    Serial.print(date[1]);
    Serial.print(",");
    Serial.println(date[2]);
  */  
    // get next address...
    addr = EEPROM.getAddress(sizeof(byte)); 
    delay(100);
    
  }
  delay(200);    
}



void loop() 
{
 
}


