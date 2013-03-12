/*
 * HABS (Halfluck Automated Brewing System)
 * An open-source Brewbot for Arduino. 
 * 
 * Project Started August 2008
 * 
 * (cc) by Rob Hart, Paul Walker
 *
 * http://www.halfluck.com
 * http://creativecommons.org/license/cc-gpl
 *
 * Compiled & Tested in Arduino 1.0.1
 * using OneWire Library http://www.arduino.cc/playground/Learning/OneWire
 * using Button Library http://www.arduino.cc/playground/Code/Button
 */

// Added by Paul Walker on the Eurostar to Paris 13/10/2008 :-)
// Added by Paul Walker at Heathrow T4, London, UK 13/11/2008 :-)

#include <Button.h>
#include <OneWire.h>
#include <EEPROM.h>

Button button1 = Button(2);       // Button 1
Button button2 = Button(3);       // Button 2
Button button3 = Button(4);       // Button 3
Button button4 = Button(5);       // Button 4
Button floatswitch = Button(6);   // Float Switch

const byte PinTemp = 7;                  // TEMP SENSORS
const byte PinSpeaker = 8;               // SPEAKER

const byte PinElementHlt = 9;            // SSR FOR HLT/KETTLE ELEMENT
const byte PinPumpHlt = 10;              // SSR FOR PUMP 1 (HLT TO MASHTUN) 
const byte PinPumpMT = 11;               // SSR FOR PUMP 2 (MASH TUN TO HLT/KETTLE)
const byte PinMotorMT = 12;              // SSR FOR MASH STIRRER MOTOR

//byte PinLed = 13;                      // HAS THE STANDARD LED ON IT, USED FOR MONITORING UPLOAD OF SKETCH

//byte Pin14 = 14;              //  **SPARE** 
//byte Pin15 = 15;              //  **SPARE** 
//byte Pin16 = 16;              //  **SPARE** 
//byte Pin17 = 17;              //  **SPARE** 
//byte Pin18 = 18;              //  **SPARE** 
//byte Pin19 = 19;              //  **SPARE** 

byte sensor1[8] = {0x28, 0xF8, 0x58, 0x6C, 0x01, 0x00, 0x00, 0x4F};       // User Defined Temperature Sensor Location
byte sensor2[8] = {0x28, 0x1E, 0x3E, 0x6C, 0x01, 0x00, 0x00, 0x59};       // User Defined Temperature Sensor Location

const byte totalsteps = 13; // total number of steps in entire process

const char versionnumber[5] = "1.30";     // this is the current version number of the program

byte testmode;  
int countdowntimer; 
byte checkfloat;
int striketemp;   //  set to 0 if you want the strike temp automatically calculated otherwise manually set your strike temp
int targetmashtemp;  // this is for the desired mash temp NOTE: Ignored if "striketemp" has anyother figure than 0
int fudgefactor; // // "Fudge Factor" to account for thermal mass of vessel (1.7 deg C is typical) NOTE: Ignored if "striketemp" has anyother figure than 0 //TODO support decimal places
int graintowaterratio;          // Mash to Water Ratio (3 is typical)  NOTE: Ignored if "striketemp" has anyother figure than 0 //TODO support decimal places
byte spargetemp;
byte doughinpumpruntime; // runs dough in pump for desired time 
byte mashlength;     // variable, how long to mash? 
byte mashoutpumpruntime;    // runs dough in pump for desired time 
byte mashoutstirtime;  // how long to stir for before mash rest 
byte mashoutresttime;  // pause after sparge 
byte vorlaufpumpruntime;    // runs mash hlt pump for desired time  
byte fillkettlepumpruntime;    //how long to run the pump from mlt to kettle to fill after the mash/vorlauf 
byte boiltemp;     // variable, what temp to get to before start boil timer 
byte boillength;     // variable, how long to boil? 
byte hopaddition1;    // first hop addition time
byte hopaddition2;   //  second hop addition time
byte hopaddition3;    //  third hop addition time
byte endofboilresttime;  // rest after boil 

long striketime = 0;    // this is used to store how long it took for the hlt to get to strike temp, this is in milliseconds
long boiltime = 0;      // this is used to store how long it took for the kettle to get to boil, this is in milliseconds
int mashtemp5 = 0;     // grab the mash temp value 10 minutes in
int mashtemp10 = 0;     // grab the mash temp value 10 minutes in
int mashtemp30 = 0;     // grab the mash temp value 30 minutes in 
int mashtempend = 0;    // grab the mash temp value at the end of mashing
int mashouttemp = 0;    // grab the mash temp value once started mash rest

int step = 1;            // start at step 1
int currenttemp1 = 0;     // curent temperature (1/16th degrees) sensor1
int currenttemp2 = 0;     // curent temperature (1/16th degrees) sensor2
int lasttemp1    = 0;     // previous temperature sensor 1
int lasttemp2    = 0;     // previous temperature sensor 2

byte currenttemp1byte;

OneWire ds(PinTemp);  // temp sensor

//extern volatile unsigned long timer0_millis;   //used to reset internal timer millis

unsigned long previous_millis_value = 0;      ///this stuff for elapsed time
unsigned long cumulativeSeconds = 0;
unsigned int totalseconds = 0;
unsigned int totalminutes = 0;
unsigned int totalhours = 0;
char totaltime[9] = "  :  :  ";
char lasttotaltime[9] = "  :  :  ";
char stepelapsedtime[9] = "  :  :  ";
unsigned long cyclecount=0;
char steptotaltime[9] = "  :  :  ";
char temptime[9] = "  :  :  ";

unsigned long startstep = 0;  // placeholder for the starting of step time
unsigned long stopstep = 0;  // placeholder for the stop of step time
unsigned long steptime = 0; // time in milliseconds required for this step
const unsigned long steptime5 = (unsigned long)5*60*1000; // calculate when to record mash temperature @ 10 mins
const unsigned long steptime10 = (unsigned long)10*60*1000; // calculate when to record mash temperature @ 10 mins
const unsigned long steptime30 = (unsigned long)30*60*1000; // calculate when to record mash temperature @ 30 mins
unsigned long stopstep5; // end time for recording mash temperature @ 10 mins
unsigned long stopstep10; // end time for recording mash temperature @ 10 mins
unsigned long stopstep30; // end time for recording mash temperature @ 30 mins
unsigned long startmashmotor; // for mash stirrer motor
byte stopstep5reached = false; //has the 10 min stop point been reached?
byte stopstep10reached = false; //has the 10 min stop point been reached?
byte stopstep30reached = false; //has the 10 min stop point been reached?
byte startmashmotorreached = false; //has the mash motor started?

unsigned long hopadditionbeepstart1 = 0; // time in milliseconds required for this step
unsigned long hopadditionbeepstop1 = 0; // time in milliseconds required when to stop beep step
unsigned long hopadditiontime1 = 0; // time in milliseconds required for this step
unsigned long hopadditionbeepstart2 = 0; // time in milliseconds required for this step
unsigned long hopadditionbeepstop2 = 0; // time in milliseconds required when to stop beep step
unsigned long hopadditiontime2 = 0; // time in milliseconds required for this step
unsigned long hopadditionbeepstart3 = 0; // time in milliseconds required for this step
unsigned long hopadditionbeepstop3 = 0; // time in milliseconds required when to stop beep step
unsigned long hopadditiontime3 = 0; // time in milliseconds required for this step

long stepelapsed = 0;  // number of milliseconds remaining
int stepelapsedsecs = 0;  // number of seconds reamaining
int timercalculated = 0; // has the timer for a timed step been set (initially = false)

int temparray[16] = {
  0,1,1,2,3,3,4,4,5,6,6,7,8,8,9,9}; // temperature lookup array

byte ReadPinElementHlt;       //placeholder for pin state  
byte ReadPinPumpHlt;          //placeholder for pin state 
byte ReadPinPumpMT;           //placeholder for pin state 
byte ReadPinMotorMT;          //placeholder for pin state 

int incoming = 0;             // placeholder for serial read stuff 

void setup()                    // run once, when the sketch starts
{
  pinMode(PinSpeaker, OUTPUT);      // sets the digital pin as output
  pinMode(PinTemp, INPUT);          // sets the digital pin as input

  pinMode(PinElementHlt, OUTPUT);    // sets the digital pin as output
  pinMode(PinPumpHlt, OUTPUT);      // sets the digital pin as output
  pinMode(PinPumpMT, OUTPUT);      // sets the digital pin as output
  pinMode(PinMotorMT, OUTPUT);     // sets the digital pin as output
  //  pinMode(Pin13, OUTPUT);         // sets the digital pin as output

  //  pinMode(Pin14, OUTPUT);      //sets the analog pin (in digital mode) as output
  //  pinMode(Pin15, OUTPUT);      //sets the analog pin (in digital mode) as output
  //  pinMode(Pin16, OUTPUT);      //sets the analog pin (in digital mode) as output
  //  pinMode(Pin17, OUTPUT);      //sets the analog pin (in digital mode) as output
  //  pinMode(Pin18, OUTPUT);      //sets the analog pin (in digital mode) as output
  //  pinMode(Pin19, OUTPUT);      //sets the analog pin (in digital mode) as output

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
  ds.write(0x44,0);	   // start conversion, with parasite power on at the end
  //ds.write(0x44,1);	   // start conversion, with parasite power on at the end

 // delay(800);     // maybe 750ms is enough, maybe not

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);	   // Read Scratchpad


  for ( i = 0; i < 9; i++) {	     // we need 9 bytes
    data[i] = ds.read();

  }
  Temp=(data[1]<<8)+data[0]; //take the two bytes from the response relating to temperature

  return Temp;
}

void Core()
{
    ReadPinElementHlt = digitalRead(PinElementHlt);

    if (checkfloat == 1)
    { 
      if ((ReadPinElementHlt == HIGH) && (floatswitch.isPressed()))
      {
        digitalWrite(PinElementHlt, LOW);
        Serial.print("?x12?y2E");  // set element ssr to E for "Error" which is off
      }
    }
    while(millis() - previous_millis_value >= 1000)
    {
      cumulativeSeconds++;
      previous_millis_value += 1000;
    }
    formattimeseconds(cumulativeSeconds,totaltime);

    if (totaltime!=lasttotaltime)  // only update if changed to prevent flooding
    {
      Serial.print("?x00?y2tot");              
      delay(100);
      Serial.print(totaltime);
      lasttotaltime==totaltime;
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
      Serial.print("?x01?y3HK=");
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
      Serial.print("?x11?y3MT=");
      Serial.print(currenttemp2>>4);
      byte overflow = currenttemp2 % 16;
      byte decimalvalue = temparray[overflow]+48;
      Serial.print("."); 
      Serial.print(decimalvalue); 
      Serial.print("?0"); //deg C 
      lasttemp2 = currenttemp2;
    }
        if (Serial.available() > 0)
    {
      incoming = Serial.read();
    }
    else
    {
      incoming = 0;
    }

 }
//  http://www.phy.mtu.edu/~suits/notefreqs.html
void beep (unsigned char PinSpeaker, int frequencyInHertz, long timeInMilliseconds)     // the sound producing function
{ 	 
  int x; 	 
  long delayAmount = (long)(1000000/frequencyInHertz);
  long loopTime = (long)((timeInMilliseconds*1000)/(delayAmount*2));
  for (x=0;x<loopTime;x++) 	 
  { 	 
    digitalWrite(PinSpeaker,HIGH);
    delayMicroseconds(delayAmount);
    digitalWrite(PinSpeaker,LOW);
    delayMicroseconds(delayAmount);
  } 	 
} 

void welcomescreen()
{
  Serial.print("?c0"); // turn cursor off
  delay(100);
  Serial.print("?D01818000704040700");        //define custom character degrees celcius 
  delay(100);
  Serial.print("?f");
  delay(50);
  Serial.print("?x01?y0Halfluck Automated");
  delay(50);
  Serial.print("?x03?y1Brewing System");            // this is the initial display screen (LCD)
  delay(50);
  Serial.print("?x07?y2v");            // this is the initial display screen (LCD)
  delay(50);
  Serial.print(versionnumber);                // grabs the current version number
  delay(50);
  Serial.print("?x02?y3www.halfluck.com");            // this is the initial display screen (LCD)
  beep(PinSpeaker,2093,500); 	     //C: play the note C (C7 from the chart linked to above) for 500ms
  Serial.print("?g");                //this will beep the remote speaker thru Screen Emulator Program on PC
}

void mainmenu()
{
  Serial.print("?f");
  delay(50);
  Serial.print("?x01?y01 for Manual Mode");
  delay(50);
  Serial.print("?x01?y12 for Auto Mode");
  delay(50);
  Serial.print("?x01?y23 for CIP Mode");
  delay(50);
  Serial.print("?x01?y34 for Setup Menu");
  delay(50);
}

void formattime(int hours, int mins, int secs, char time[]) { // format a time to a string from hours, mins, secs
  // PW 20090203 Added support for overflow of mins and secs
  if (secs>60) {
    secs=secs-60;
    mins++;
  }
  if (mins>60) {
    mins=mins-60;
    hours++;
  }
  time[2]=':';
  time[5]=':';
  time[0]=48+(hours / 10);
  time[1]=48+(hours % 10);
  time[3]=48+(mins / 10);
  time[4]=48+(mins % 10);
  time[6]=48+(secs / 10);
  time[7]=48+(secs % 10);
}

void formattimeseconds(long secs, char time[]) { // format a time to a string from seconds only
  int tempsecs = secs % 60;
  int tempmins = (secs / 60) % 60;
  int temphours = secs  / 3600;
  formattime(temphours,tempmins,tempsecs,time);

}

void calcstrike()    // used for calculating the strike temperature of the brewing liquor pre mash
{
 
 int tempofgrain = currenttemp2>>4;  // Temperature of grain, this is grabbed via the tempature sensor inside the Mashun
 //byte tempofgrain = (byte)(currenttemp2>>4); // Temperature of grain, this is grabbed via the tempature sensor inside the Mashun
  
 striketemp = (targetmashtemp + 0.4 * (targetmashtemp - tempofgrain) / graintowaterratio) + fudgefactor;   
  
  // can probably make the variables bytes (actually will have to) then grab currenttemp2 and convert to byte
  // may need to do some rounding on this e.g.  77.4*c would be 77 and 77.8*c would be 78
}

void loop()                     // start of actual sketch, run over and over again
{
  // The code below displays a text screen and waits for user intervention (either select or start on keypad)

  delay (7000);     //delays 7 seconds waiting for lcd to boot up
  welcomescreen();    // calls void welcomescreen()
  delay(3000);
  mainmenu();

  while (1) 
  {
    if (Serial.available() > 0)
    {
      incoming = Serial.read();
    }
    else
    {
      incoming = 0;
    }
    if ((button1.uniquePress()) || ((char)incoming == '1'))
    {
      delay(500);     
      manualmode();
    }
    if ((button2.uniquePress()) || ((char)incoming == '2'))
    {
      delay(500);     
      automode();
    }
    if ((button3.uniquePress()) || ((char)incoming == '3'))
    {
      delay(500); 
      cipmode();
    }
        if ((button4.uniquePress()) || ((char)incoming == '4'))
    {
      delay(500); 
      setupmenu();
      delay(100);
      mainmenu();
    }
  }
}



