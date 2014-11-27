//#include <JeeLib.h>
#include <Ports.h>

//#include <RF12.h>
//#include <RF12sio.h>
//#include <RF69.h>
//#include <RF69_avr.h>
//#include <RF69_compat.h>
#include <JeeLib.h>
#include <PortsLCD.h>

/// Hooking up a DS1307 (5V) or DS1340Z (3V) real time clock via I2C.
// Reading sensor data (from Analog1) and writing to the built-in EEPROM on the ATMEL chip
// with a timestamp (h,m,s) for each data byte.  To be read back using the "datalogger_reader" sketch.
// Using an LCD for display. http://oomlout.com/parts/LCDD-01-guide.pdf
//
// 2014 Nov 19 by Evan Raskob <e.raskob@rave.ac.uk> http://opensource.org/licenses/mit-license.php

// the real-time clock is connected to port 1 in I2C mode (AIO = SCK, dIO = SDA)
// See JeeLib for info on Ports : http://jeelabs.org/2011/11/10/pins-damned-pins-and-jeenodes/#comments 
// EEPROMex library from http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino
//
// todo: hit a button (to reset the time) 


#include <EEPROMex.h>
#include <EEPROMVar.h>
//#include <LiquidCrystal.h>


//#define _EEPROMEX_DEBUG //comment out for production code...


PortI2C myport (1 /*, PortI2C::KHZ400 */);  // RTC port - see http://jeelabs.org/2009/09/16/jeenode-pinout/
DeviceI2C rtc (myport, 0x68);  // RTC device driver

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 6, 7, 8);

/*
 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 6
 * LCD D6 pin to digital pin 7
 * LCD D7 pin to digital pin 8
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3 on the LCD)
 */

//
// time variables
//
byte now[6]; // current time
byte before[6]; // time we started logging data

const byte hourIndex = 3; // index in date array of hour byte
const byte minIndex = 4;  // etc
const byte secIndex = 5;  // etc

const int MINS_PER_HOUR = 60;
const int SECS_PER_MIN = 60;


int nextAddr; // next address to write data to


// functions - defined at end of file /////////////////////////
static byte bin2bcd (byte val);
static byte bcd2bin (byte val);
boolean gtDate( byte* dl, byte* dr );
void printDate(byte *d, byte length=6);
static void getDate (byte* buf);
static void setDate (byte yy, byte mm, byte dd, byte h, byte m, byte s);
int diffSecs( byte* dl, byte* dr );
void resetBaseAddress();
void saveSensorData(byte data);
///////////////////////////////////////////////////////////////



//
// Overwrite this to get your own sensor data (as a byte) to log
//

byte getSensorData()
{
  (byte) (analogRead(A1) / 4);
}


void setup() {
  Serial.begin(57600);
  Serial.println("\n[data_writer]");

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");



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


void loop() 
{
  getDate(now);
    
  // read some data
  byte data = getSensorData();

  printDate(now);
  Serial.print("data: " );
  Serial.println(data);

  int timeDiffSecs = diffSecs(now,before);
  Serial.print("time diff in secs: " );
  Serial.println(timeDiffSecs);


  //saveSensorData(data);


  delay(2000);
}



void saveSensorData(byte data)
{
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
}




void resetBaseAddress()
{
    EEPROM.updateInt(0,0);
    EEPROM.setMemPool(nextAddr, EEPROMSizeUno); // top 2 bytes (an int) are for the nextAddr
    nextAddr = EEPROM.getAddress(sizeof(byte));
}


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

