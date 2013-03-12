
#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h> 

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
Servo myservo;
float tempUpper;
float tempLower;
float avgTemp1;
//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,2,5,1, DIRECT); //these tuning parameters NEED to be updated for 5gal brew
DeviceAddress upperThermometer = { 0x28, 0x4C, 0xB6, 0x9A, 0x04, 0x00, 0x00, 0xA1 };
DeviceAddress lowerThermometer = { 0x28, 0x3F, 0xF2, 0x9A, 0x04, 0x00, 0x00, 0x96 };



void setup  ()
{
  //Input = avgTemp1(0);
  Setpoint = 160; //temperature to set to in degrees F 
  myPID.SetMode(AUTOMATIC);
  //myPID.SetOutputLimits(10, 95);//10 and 95 are the max and min servo angle positions
  myPID.SetOutputLimits(650, 1520);//650 and 1520 are the write microseconds limits
  myPID.SetSampleTime(2000);
  Serial.begin(9600);
  myservo.attach(8);  //the pin for the servo control 
    myservo.write(60); //set initial servo position if desired
  //Serial.println("PID to servo control angle"); // so I can keep track of what is loaded
  Serial.println("Time, Set Point, Upper Temp, Lower Temp, Avg Temp, Output Angle"); //headers for CSV table output

   sensors.begin();
  // set the resolution to 10 bit (good enough?), going to 12 for greater precision
    sensors.setResolution(upperThermometer, 12);
    sensors.setResolution(lowerThermometer, 12);

}


void loop (void)
{
  delay(2000);
 // Serial.print("Time is: "); comment out if using excel
  Serial.print(millis());Serial.print(", ");
  //Serial.print("Setpoint: ");
  Serial.print(Setpoint); Serial.print(", ");
      //Serial.println("Getting temperatures...");
       //Temperature sensor section
     sensors.requestTemperatures();
      tempUpper = sensors.getTempF(upperThermometer); //Use sensors.getTempC if you prefer your systems metric
      tempLower = sensors.getTempF(lowerThermometer); 
      avgTemp1 = (tempUpper+tempLower)/2;
  //Serial.print("upper temperature is: ");
  Serial.print(tempUpper);Serial.print(", ");
 // Serial.print("lower temperature is: ");
  Serial.print(tempLower);Serial.print(", ");
 // Serial.print("avg pot temperature is: ");
  Serial.print(avgTemp1); Serial.print(", ");   
  
  //PID section  
  Input =  avgTemp1;
  myPID.Compute();
  //Servo section
  //int invOutput = map(Output, 10, 95, 95, 10); //need to map 10 to 95 to reverse the output from the PID
 // Since 10 is max value on the gas and 95 is min
  int invOutput = map(Output, 650, 1520, 1520, 650);
  //Serial.print("writing angle: "); 
  Serial.println(Output);
  //myservo.write(invOutput);
  myservo.writeMicroseconds(invOutput);
  
  
}

