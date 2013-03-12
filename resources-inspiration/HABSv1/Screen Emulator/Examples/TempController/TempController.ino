 /*
 * Halfluck Temperature Controller (for Fermentation or a Kegerator)
 * An open-source Temperature Controller, Monitor & Logger for Arduino. 
 * 
 * TempController.pde v1.2
 *
 * (cc) by Rob Hart
 *
 * http://www.halfluck.com
 * http://creativecommons.org/license/cc-gpl
 *
 * Compiled & Tested in Arduino 0017 Alpha
 * using OneWire Library (http://www.arduino.cc/playground/Learning/OneWire)
 * using Screen Emulator PC Program (http://www.halfluck.com/source-code)
 *
 * Buttom 1 = Target Temp - UP
 * Button 2 = Target Temp - DOWN
 * Button 3 = Lock/Unlock Buttons
 * Button 4 = Refresh Screen (used if you start Screen Emulator after the Arduino)
 * 
 */

#include <OneWire.h>
#include <EEPROM.h>

const byte PinCool = 5;               // Pin for Fridge
const byte PinHeat = 7;               // Pin for Heat Belt
const byte PinTemp = 9;               // 2 x Dallas Ds1820 Temperature Sensors on Digital Pin 9

const byte CompressorDelayTime = 5;   //  (5 minutes is 1000 x 60 x 5 = 300000 ) 
byte CompressorDelayTimeCalculate = 0;   //  
unsigned long CompressorOffTime = 0;     // placeholder for when compressor has just switched off
unsigned long CompressorNextOnTime = 0;  // placeholder for calculated time the compressor is allowed back on

// The compressor time delay is to prevent the compressor from cycling back on again less than 5 minutes after it has shut off. 
// The 5 minutes allows the pressures in the system to equalize and then the compressor doesn't start under a load. 
// This time delay radically reduces the chance of a compressor not starting (due to an overload).

byte TargetTemp;                      // this is the placeholder for the target temperature
byte LockButtons = 0;                 // Lock / Unlock Buttons
const byte HeatTemp = 2;              // How many *c below targettemp before heattemp kicks in (DEFAULT 2)
const byte CoolTemp = 0;              // How many *c above targettemp before cooltemp kicks in (DEFAULT 1)

byte sensor1[8] = {0x28, 0xB3, 0x5E, 0x6C, 0x01, 0x00, 0x00, 0x5E};   // User Defined Temperature Sensor Address 1
byte sensor2[8] = {0x28, 0xE8, 0xD7, 0xE1, 0x01, 0x00, 0x00, 0xF4};   // User Defined Temperature Sensor Address 2

int currenttemp1 = 0;     // curent temperature (1/16th degrees) sensor1
int currenttemp2 = 0;     // curent temperature (1/16th degrees) sensor2
int lasttemp1    = 0;     // previous temperature sensor 1
int lasttemp2    = 0;     // previous temperature sensor 2

byte currenttemp1byte;

OneWire ds(PinTemp);  // temp sensor 1 on pin 9

int temparray[16] = {
  0,1,1,2,3,3,4,4,5,6,6,7,8,8,9,9}; // temperature lookup array

void setup()                    // run once, when the sketch starts
{

  pinMode(PinHeat, OUTPUT);          // sets the digital pin as input
  pinMode(PinCool, OUTPUT);          // sets the digital pin as input
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
  TargetTemp = EEPROM.read(0);
  delay(100);
  Serial.print("?c0"); // turn cursor off
  delay(100);
  Serial.print("?D01818000704040700");        //define custom character degrees celcius 
  delay(100);
  Serial.print("?f 3 TEMP CONTROLLER");
  delay(100);
  Serial.print("?x00?y1TARGT");
  delay(100);
  Serial.print("?x06?y1");
  delay(100);
  Serial.print(TargetTemp, DEC);
  delay(100);
  Serial.print("?0"); 
  delay (100);
  Serial.print("?x11?y1UNLOCKED");
 
  CompressorOffTime = millis();    // if the temp is above high range then cool ssr will be switched on when sketch starts

  while (1) 
  {
    if ((currenttemp1byte<=TargetTemp-HeatTemp) && (currenttemp1byte>0)) // below low range
    {
      digitalWrite(PinHeat, HIGH);       // turn heating on
      Serial.print("?x01?y2HEAT ON ");
      digitalWrite(PinCool, LOW);       // turn cooling off
      Serial.print("?x11?y2COOL OFF");
    }
    else if ((currenttemp1byte>TargetTemp-HeatTemp) && (currenttemp1byte<TargetTemp+CoolTemp) && (currenttemp1byte>0)) //ideal range
    {
      digitalWrite(PinHeat, LOW);       // turn heating off
      Serial.print("?x01?y2HEAT OFF");
      digitalWrite(PinCool, LOW);       // turn cooling off
      Serial.print("?x11?y2COOL OFF");

    if (CompressorDelayTimeCalculate == 1)    
    {
      CompressorOffTime = millis();    // grab the time the compressor went off and store it
      CompressorNextOnTime = (CompressorOffTime+((unsigned long)CompressorDelayTime*60*1000));  // calculate when the compressor is allowed to come on next
      CompressorDelayTimeCalculate = 0;
    }
      
    } else if ((currenttemp1byte>=TargetTemp+CoolTemp) && (currenttemp1byte>0)) // above high range
    {
      if (millis() >= CompressorNextOnTime)     // if its been "CompressorDelayTime" since the compressor was last off????
      {
      digitalWrite(PinHeat, LOW);       // turn heating off
      Serial.print("?x01?y2HEAT OFF");
      digitalWrite(PinCool, HIGH);       // turn cooling on
      Serial.print("?x11?y2COOL ON ");
      CompressorDelayTimeCalculate = 1;
      }
     
    }
    
    if (millis() < CompressorOffTime)   // this is for the millis rollover 
    CompressorOffTime = 0;


    if (Serial.available() > 0)
  {
    int incoming = Serial.read();

    if (((char)incoming == '1') && (LockButtons == 0))
    {
      TargetTemp++;        // increment value
      EEPROM.write(0, TargetTemp);   // this writes the value "TargetTemp" into eeprom position "0" 
      delay(100);
      Serial.print("?x06?y1....");   //clear 3 digits space 
      delay(100);
      Serial.print("?x06?y1");      // put cursor here
      delay(100);
      Serial.print(TargetTemp, DEC); // print value on screen
      delay(100);
      Serial.print("?0"); 
     } 
    if (((char)incoming == '2') && (LockButtons == 0))
    {
      TargetTemp--;        // increment value
      EEPROM.write(0, TargetTemp);   // this writes the value "TargetTemp" into eeprom position "0" 
      delay(100);
      Serial.print("?x06?y1....");
      delay(100);
      Serial.print("?x06?y1");
      delay(100);
      Serial.print(TargetTemp, DEC);
      delay(100);
      Serial.print("?0"); 
    } 
    if ((char)incoming == '3')
    {
      if (LockButtons == 0)
      {
      Serial.print("?x11?y1*LOCKED*");
      LockButtons = 1;
      }
      else
      {
      Serial.print("?x11?y1UNLOCKED");
      LockButtons = 0;
    }
    } 
    if (((char)incoming == '4') && (LockButtons == 0))
    {
    delay(100);
    Serial.print("?f 3 TEMP CONTROLLER");
    delay(100);
    Serial.print("?x00?y1TARGT");
    delay(100);
    Serial.print("?x06?y1");
    delay(100);
    Serial.print(TargetTemp, DEC);
    delay(100);
    Serial.print("?0"); 
    delay (100);
    if (LockButtons == 1)
    {
    Serial.print("?x11?y1*LOCKED*");
    }
    else
    {
    Serial.print("?x11?y1UNLOCKED");
    }
    } 
  }

    currenttemp1 = tempread(sensor1);
    if (currenttemp1>0) {
      currenttemp1byte = (byte)(currenttemp1>>4);
    } 
    else {
      currenttemp1byte = 0;
    }

    if ((currenttemp1 != lasttemp1) && (currenttemp1>0))
    {
      Serial.print("?x01?y3T1=");
      Serial.print(currenttemp1>>4);
      byte overflow = currenttemp1 % 16;
      byte decimalvalue = temparray[overflow]+48;
      Serial.print("."); 
      Serial.print(decimalvalue); 
      Serial.print("?0"); //deg C
      Serial.print("  ");             
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
      Serial.print(" ");            
      lasttemp2 = currenttemp2;
    } 
  }
}





