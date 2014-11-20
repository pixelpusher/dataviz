/// Hooking up a DS1307 (5V) or DS1340Z (3V) real time clock via I2C.
// Reading sensor data (from Analog1) and writing to the built-in EEPROM on the ATMEL chip
// with a timestamp (h,m,s) for each data byte.  To be read back using the "datalogger_reader" sketch.
//
// 2014 Nov 19 by Evan Raskob <e.raskob@rave.ac.uk> http://opensource.org/licenses/mit-license.php

// the real-time clock is connected to port 1 in I2C mode (AIO = SCK, dIO = SDA)
// See JeeLib for info on Ports : http://jeelabs.org/2011/11/10/pins-damned-pins-and-jeenodes/#comments 
// EEPROMex library from http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino
//
// todo: hit a button (to reset the time) 

#include <JeeLib.h>
#include <EEPROMex.h>
#include <EEPROMVar.h>

//#define _EEPROMEX_DEBUG //comment out for production code...


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


void printDate(byte *d, byte length=6)
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
  Serial.println("\n[data_writer]");

  // Set maximum allowed writes to maxAllowedWrites. 
  // More writes will only give errors when _EEPROMEX_DEBUG is set
  EEPROM.setMaxAllowedWrites(10000);

  //lastWrittenAddr = EEPROM.getAddress(sizeof(int));

  #define _CLEAR_ADDR_

  #ifdef _CLEAR_ADDR_
    EEPROM.updateInt(0,0);
    nextAddr = 2;
  #else
    // read last address... this one should always be empty as it gets written AFTER the data is written
    nextAddr = EEPROM.readInt(0);
  #endif

  EEPROM.setMemPool(nextAddr, EEPROMSizeUno); // top 2 bytes (an int) are for the nextAddr

  // test code:
  //setDate(14, 11, 19, 12, 54, 00);
   
    nextAddr = EEPROM.getAddress(sizeof(byte));
   
  getDate(before);
  delay(200);  
}


void resetBaseAddress()
{
    EEPROM.updateInt(0,0);
    EEPROM.setMemPool(nextAddr, EEPROMSizeUno); // top 2 bytes (an int) are for the nextAddr
    nextAddr = EEPROM.getAddress(sizeof(byte));
}


void loop() 
{
  getDate(now);
    
  // read some data
  byte data = (byte) (analogRead(A1) / 4);

  printDate(now);
  Serial.print("data: " );
  Serial.println(data);

  int timeDiffSecs = diffSecs(now,before);
  Serial.print("time diff in secs: " );
  Serial.println(timeDiffSecs);

  // save it
  Serial.print("writing to addr: " );
  Serial.println(nextAddr);
  
  while(!EEPROM.isReady());
  EEPROM.update(nextAddr, data);
  
  // write time
  nextAddr = EEPROM.getAddress(sizeof(byte)*3);
  while(!EEPROM.isReady());
  EEPROM.updateBlock<byte>(nextAddr, &now[3], 3); // only write last 3 bytes (h m s)
  while(!EEPROM.isReady());

  Serial.print("writing to addr: " );
  Serial.println(nextAddr);

  // next address...
  nextAddr = EEPROM.getAddress(sizeof(data));
  EEPROM.updateInt(0,nextAddr); // write next open address
  while(!EEPROM.isReady());
  /*
  if (!result) 
  {
    Serial.println("data filled");
  }
  */


  delay(2000);
}



