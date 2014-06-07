/*
Things that matter:
 This project was created by James Rubinstein for the purpose of building an
 automated brew-in-a-bag (BIAB) rig for brewing ~5 gallons of beer. 
 The basic idea is to read a beerxml file and use that to create a set of instructions
 for the arduino. Using serial to collect data, provide manual control, and automation override
 we'll probably start by hard-coding values :)
 
 This project and it's code are released as open source, attribution only. 
 
 This is the work of amateurs, and you should be careful.
 Playing with high voltages, propane, and electricity can get you killed. 
 
 
 Now let's get started
 *Wire up power to the arduino - 5v 
 *Onewire .................................Pin 12
 *Servo for gas control to ................Pin 8
 *Solid State Relay (SSR) for ignitor to...Pin 4
 *Motor for pump (if so equipped)..........Pin 10
 *Thermocouple ............................Pin A0
 *Flame sensor (using the analog out)......Pin A2
 *Buzzer on................................Pin 9
 Use of the tone() function will interfere with PWM output on pins 3 and 11
 
 */
/********stuff you need to make the sketch work *****/
#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h> //convenience library for dealing with ds18b20
#include <Servo.h> 
#define ONE_WIRE_BUS 12 // Data wire is plugged into pin 12 on the Arduino
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature. 

/**********global constants***********/
const int thermocouplePin = A0;
const int flameSensorPin = A2;
const int ignitorPin = 4;
const int speakerPin = 9;
const int ledPin = 13;

/*********recipe goes here in these arrays******/
const String steps[] = {
  "Strike", "Mash1", "Mash2", "Boil","Done"}; //needs to have a done stage last for safety
const float temps[] = {  //temps in degrees F
  100,90,95,105,0};
const float times[]={  //times in minutes
  0.5,5,5,5,100}; //0 minutes for strike so it alarms as soon as temp is hit. may make it a few seconds so temp will stabilize
const boolean needsUserInput [] = { //boolean values to tell if there should be user intervention at the end of the step
  true, false, true, true,false};
const float boilAdditionTimes[]= { //times in minutes
  1};

/**********global variables********/
//Define global Variables we'll be connecting to
Servo gasServo;
float tempUpper;
float tempLower;
float avgTemp1;
DeviceAddress upperThermometer, lowerThermometer;

//DeviceAddress upperThermometer = { 
  //0x28, 0x4C, 0xB6, 0x9A, 0x04, 0x00, 0x00, 0xA1 }; // need to get these manually for now using
//DeviceAddress lowerThermometer = { 
  //0x28, 0x3F, 0xF2, 0x9A, 0x04, 0x00, 0x00, 0x96 }; // the one_wire_address_finder sketch. Add automation later 
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters
double Kp=14.0, Ki=27.5, Kd=2; //these tuning parameters NEED to be updated for 5gal brew
PID myPID(&Input, &Output, &Setpoint,Kp,Ki,Kd, DIRECT); 
boolean isLit = false;

unsigned long stepTime = 0;
String currentStep;
boolean inputRequired = false;
boolean timesUP = false;
boolean inputReceived = false;
unsigned long stopStep;
unsigned long startStep;
boolean timerCalculated;

int numSteps = (sizeof(steps)/sizeof(String));//number of steps
int count = 0;//counter for steps
int countAdditions = 0; //counter for the number of boil additions
int numAdditions = (sizeof(boilAdditionTimes)/sizeof(int));//??check this

/******stuff to play a tune for the alarm******/
int length = 15; // the number of notes
char notes[] = "ccggaagffeeddc "; // a space represents a rest
int beats[] = { 
  1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;

/*************begin setup*********/

void setup  ()
{
    sensors.begin();
 if (!sensors.getAddress(upperThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(lowerThermometer, 1)) Serial.println("Unable to find address for Device 1"); 

  Serial.begin(9600);
  Serial.println("Time, Step Time, Step Name, Set Point, Upper Temp, Lower Temp, Avg Temp, Output Angle, Is Lit"); //headers for CSV table output
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 100);// trying 0-100 as percent to make the code more readable 650 and 1520 are the write microseconds limits
  myPID.SetSampleTime(2000);
  gasServo.attach(8);  //the pin for the servo control 
  gasServo.writeMicroseconds(1520); //set initial servo position if desired //this is fully closed
  sensors.begin();
  sensors.setResolution(upperThermometer, 12); //12 bit precision
  sensors.setResolution(lowerThermometer, 12);
  isLit = false; //making sure that isLit is false until actually lit
  pinMode(ignitorPin, OUTPUT);     // sets the digital pin as output
  digitalWrite (ignitorPin, LOW); //start with the ignitor in off position
  pinMode(ledPin, OUTPUT); 
  pinMode(speakerPin, OUTPUT);
}

/*********recipe info*****/
void getStepInfo(){
  currentStep = steps[count];
  Setpoint = temps[count];
  stepTime = (times[count] * 60000);
  inputRequired = needsUserInput[count];
}

/******* decide to move to the next step or not********/
int nextStep(){
  if (count < numSteps){
    if (timesUP && (avgTemp1 >= (Setpoint -5)) && inputRequired && !inputReceived){
      playSong();
      inputReceivedCheck();
    }
    else if (timesUP && (avgTemp1 >= (Setpoint -5)) && inputRequired && inputReceived){
      count++;
      timesUP = false;
      inputReceived = false;
    } 
    else if (timesUP && (avgTemp1 >= (Setpoint -5)) && !inputRequired){
      count++;
      timesUP = false;
    }
    else {
    }

  }
  else {
    Setpoint = 0;
    stepTime = 10000000;
    gasServo.writeMicroseconds(1520); //set servo position fully closed

  }
}

/*********get info for hop additions, etc to the boil******/
void boilAdditions(){
  unsigned long additionMillis =  (boilAdditionTimes[countAdditions])*60000;
  if (currentStep == "Boil" && timerCalculated) {
    unsigned long addTime = (startStep + stepTime) -  additionMillis; // 
    if (millis()>=addTime) { // time to add something
      //Serial.print("time to add addition number ");
      alarmShort();
      //Serial.println(countAdditions);
      countAdditions ++;

    }
  }

}


/******** see if the time is up for that brewing step ****/
boolean timesUpCheck(){
  //Serial.println(timerCalculated);
  //Serial.println(startStep);

  if ((timerCalculated == false) && (avgTemp1 >= Setpoint) && !timesUP) { //begin counting step time when temp is within 1/2 degree
    startStep = millis(); // start step clock, reads milliseconds from this moment
    stopStep = startStep + stepTime; // calculate the end of step from the configuired time "hltpumpruntime"
    timerCalculated = true; // don't do again whilst in this step
  }

  if (timerCalculated == true && (millis()>=stopStep)){ // time has finished
    timesUP = true;
    timerCalculated=false; // next step's timer not yet calculated
  }
  return timesUP;
}

void inputReceivedCheck(){
  String str = "";  
   while (Serial.available()>0) {
    str = Serial.readStringUntil('\n'); 
  }
  if ( str == "next"){  
    inputReceived = true;
  }

}
/***** get user input to stop, go to next step, or start over ********/
void getInput(){
  String input = "";
  while (Serial.available()>0) {
    input = Serial.readStringUntil('\n'); 
  }

  if (input == "stop"){
    count = numSteps -1 ; //go to done stage
    Setpoint = 0;
    //gasServo.writeMicroseconds(1520); //set servo position fully closed
  }

  if ( input == "step"){ 
    count++; //go to next stage
  }

  if (input == "start over"){
    count = 0 ; //go to first stage
  }

}

/*** alarm section shamelessly stolen from "melody" tutorial ***/

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}
void playNote(char note, int duration) {
  char names[] = { 
    'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'         };
  int tones[] = { 
    1915, 1700, 1519, 1432, 1275, 1136, 1014, 956         };

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}



void playSong() {
  for (int i = 0; i < length; i++) {
    if (notes[i] == ' ') {
      delay(beats[i] * tempo); // rest
    } 
    else {
      playNote(notes[i], beats[i] * tempo);
    }

    // pause between notes
    delay(tempo / 2); 
  }
}

/****a short buzzer alarm*****/
void alarmShort(){
  for (int i = 0; i < 5; i++) {
    playTone(300, 160);
    delay(150);
  }
}


/*********begin get temperature********/
float getTemperature()
{
  sensors.requestTemperatures();
  tempUpper = sensors.getTempF(upperThermometer); //Use sensors.getTempC if you prefer your systems metric
  tempLower = sensors.getTempF(lowerThermometer); 
  avgTemp1 = (tempUpper+tempLower)/2;
  return avgTemp1; //??is this the right syntax?
}



/*******begin run PID*********/
int runPID(float avgTemp1) // ??maybe the right syntax is int runPID( float &avgTemp1) or runPID()
{
  Input =  avgTemp1;
  //sets PID to only be used if temp is 15 below setpoint to setpoint. 
  if (Input < (Setpoint-15)) {
    Output = 100;
  }
  else if (Input > Setpoint) {
    Output = 0;
  }
  else {
    myPID.Compute();
  }
  //sets output to be 0 if output is less than 26 to prevent gas leaks- the fire dies out at 24
  if (Output < 26) {
    Output = 0;
  }
  else {
    Output = Output;
  }
  return Output;
}

/************check if gas is lit *******/
boolean isGasLit()
{
  double thermocouple = analogRead(thermocouplePin);
  int flameSensor = analogRead(flameSensorPin);
  if (thermocouple > 1.5||flameSensor < 1000) {
    isLit = true;
  }
  else { 
    isLit = false;
  }
  Serial.println(thermocouple);
   Serial.println(flameSensor);
  return isLit;
}

/*********** Light the gas routine ***********/
void lightGas()
{
  digitalWrite (ignitorPin, HIGH);
  digitalWrite (ledPin,HIGH);
  delay (1000); //make sure the ignitor stays lit for 2 seconds, should actually last for 4 seconds because of the delay in the main loop
  Output = 100; //??can I put this here ??tune time and output for best lighting behavior. >=26 is gas on
  int invOutput = map(Output, 0, 100, 1520, 650);
  gasServo.writeMicroseconds(invOutput);
  delay (4000);
  digitalWrite (ignitorPin, LOW);
  digitalWrite (ledPin,LOW);
}

/******* set servo output **********/
void moveServo (double Output)
{
  int invOutput = map(Output, 0, 100, 1520, 650); //don't forget this has to be backwards 
  if (isLit == true){
    gasServo.writeMicroseconds(invOutput);
  }
  else if (isLit == false && Output != 0 ) {
    lightGas();
    gasServo.writeMicroseconds(invOutput);
  }
  else if (isLit == false && Output == 0) {
  }
  else {
    Serial.println("Problem with gas!!!!!");//need to check this logic 
    alarmShort();
  }
}


/******** print things to serial output *****/
void printInfo()
{
  //Serial.println("Time, Step Time, Step Name, Set Point, Upper Temp, Lower Temp, Avg Temp, Output Angle, Is Lit"); //headers for CSV table output
  Serial.print(millis());
  Serial.print(", ");
  Serial.print(startStep);
  Serial.print(", ");
  Serial.print(currentStep);
  Serial.print(", ");
  Serial.print(Setpoint); 
  Serial.print(", ");  
  Serial.print(tempUpper);
  Serial.print(", ");
  Serial.print(tempLower);
  Serial.print(", ");
  Serial.print(avgTemp1); 
  Serial.print(", ");   
  Serial.print(Output);
  Serial.print(", ");
  Serial.println(isLit);
  //Serial.println(analogRead(thermocouplePin));
  //Serial.println( analogRead(flameSensorPin));
}



void printInfo2()//just for testing 
{
  //Serial.println("Time, Step Time, Step Name, Set Point, Upper Temp, Lower Temp, Avg Temp, Output Angle, Is Lit"); //headers for CSV table output
  Serial.println(millis());
  //Serial.println(analogRead(thermocouplePin));
  //Serial.println( analogRead(flameSensorPin));
}

/*******begin main loop*******/
void loop (void)
{ 
  delay(2000); //only run this loop once every 2 seconds. maybe switch to a while loop instead?
  isGasLit(); //is the gas lit?
  getTemperature();  //get temps
  getStepInfo();  //get the recipe information
  timesUpCheck();  //check to see if time is up, runs timers
  boilAdditions();  //check to see if it's time to add something to the boil
  nextStep();  //see if conditions are met to go to the next step
  getInput();  //has the user sent something over serial? take appropriate action if they have
  runPID(avgTemp1);  //PID section  input avg temp and output Output
  moveServo(Output);  //servo section writes the output to the servo
  printInfo();  //print things


  /*  next steps:
  *needs pulldown resistors on analog inputs ! 
   *get recipe info from python? Processing? 
   *ux for serial interface in python? Processing
   */


  //end
}


