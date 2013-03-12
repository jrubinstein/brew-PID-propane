/*

 Screen Emulator Temperature Logger Example Sketch - 10/7/09
 Demonstrates the uses of Data Logging on "Screen Emulator" PC Program
 
 (cc) rob@halfluck.com
 
 Currently Supports the following Commands = 
 
 Serial.print("?f");               // Clear Screen
 Serial.print("?g");               // Default Windows Beep 
 Serial.print("?n");               // CRLF, carriage return & line feed, cursor at start of next line, line cleared 
 Serial.print("?x01?y1?");         // Position cursor on x column, (two characters are required), first column is column 0
                                   // Position cursor at y row, first row is row 0, one digit only (no leading zero)
 Serial.print("?C0")               // This is used as to print the Degrees Celcious Symbol
 Serial.print("?f 3");             // Start Logging
 Serial.print("?f14");             // Stop Logging 
 
 Compiled & Tested in Arduino 0016 Alpha
 
*/


#include <OneWire.h>

const byte PinTemp = 9;               // 2 x Dallas Ds1820 Temperature Sensors on Digital Pin 9

byte sensor1[8] = {0x28, 0xB3, 0x5E, 0x6C, 0x01, 0x00, 0x00, 0x5E};   // User Defined Temperature Sensor Address 1
byte sensor2[8] = {0x28, 0xE8, 0xD7, 0xE1, 0x01, 0x00, 0x00, 0xF4};   // User Defined Temperature Sensor Address 2

int currenttemp1 = 0;     // curent temperature (1/16th degrees) sensor1
int currenttemp2 = 0;     // curent temperature (1/16th degrees) sensor2
int lasttemp1    = 0;     // previous temperature sensor 1
int lasttemp2    = 0;     // previous temperature sensor 2

OneWire ds(PinTemp);  // temp sensor 1 on pin 5

int temparray[16] = {0,1,1,2,3,3,4,4,5,6,6,7,8,8,9,9}; // temperature lookup array

void setup()                    // run once, when the sketch starts
{
  pinMode(PinTemp, INPUT);          // sets the digital pin as input
  Serial.begin(9600);	      // opens serial port, sets data rate to 9600 bps
}

int tempread(byte sensoraddr[])
// error codes:
// -1 no sensors found
// -2 invalid CRC
// -3 not a DS1820
{
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  int Temp;

  if ( !ds.search(addr)) {
    ds.reset_search();
    return -1; 
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    return -2;
  }

  if ( addr[0] != 0x28) {
    return -3;
  }


  for ( i = 0; i < 8; i++) {
    addr[i]=sensoraddr[i];
  }
  ds.reset(); // pulse the pins and wait for a response to reset the DS1820
  ds.select(addr); // 0x55 (MATCH_ROM) followed by the address of the 1820 to talk to.
  ds.write(0x44,1);	   // start conversion, with parasite power on at the end

  delay(800);     // maybe 750ms is enough, maybe not

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);	   // Read Scratchpad


  for ( i = 0; i < 9; i++) {	     // we need 9 bytes
    data[i] = ds.read();

  }
  Temp=(data[1]<<8)+data[0]; //take the two bytes from the response relating to temperature

  return Temp;
}


void loop()                     // start of actual sketch, run over and over again
{

  delay (7000);     //delays 7 seconds waiting for lcd to boot up
  Serial.print("?f 3");
  delay (100);
  Serial.print("?n Temperature Logger");

  while (1) 
  {
    currenttemp1 = tempread(sensor1);
    if ((currenttemp1 != lasttemp1) && (currenttemp1>0))
    {
      Serial.print("?x01?y3T1=");
      Serial.print(currenttemp1>>4);
      byte overflow = currenttemp1 % 16;
      byte decimalvalue = temparray[overflow]+48;
      Serial.print("."); 
      Serial.print(decimalvalue); 
      Serial.print("?0"); //deg C
      Serial.print(" ");             // ADDED TO PREVENT DOUBLE *C WHEN IT GOES TO TRIPLE DIGITS THEN BACK TO DOUBLE DIGITS 
      lasttemp1 = currenttemp1;
    }

    currenttemp2 = tempread(sensor2);
    if ((currenttemp2 != lasttemp2) && (currenttemp2>0))
    {
      Serial.print("?x11?y3T2=");
      Serial.print(currenttemp2>>4);
      byte overflow = currenttemp2 % 16;
      byte decimalvalue = temparray[overflow]+48;
      Serial.print("."); 
      Serial.print(decimalvalue); 
      Serial.print("?0"); //deg C 
      lasttemp2 = currenttemp2;
    } 
  }
}




