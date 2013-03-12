#include <Servo.h>
#include <PID_Beta6.h>

int sensorPin = 0;     // Analogue input.
int meterPin = 6;      // Meter.
int ledPin = 13;       // LED (on board).
int sensorValue = 0;   // Analogue value read.

int TRIGGERLEVEL = 50;

double Input, Output, Setpoint;
unsigned long milliSec;
unsigned long lastTime;
unsigned int timeDiff;
unsigned int rpm;
int val;
int count;

Servo myservo;
PID myPID(&Input, &Output, &Setpoint, .5, 1, 1 );


void setup() {
    Serial.begin(9600);
    myservo.attach(9);
  myservo.write(90);
    Setpoint = 333;
  myPID.SetOutputLimits( 45, 135 );
  myPID.SetMode(AUTO);
    count = 0;
    delay(1000);

}

void loop()
{
  if ( (millis() - lastTime) > 1000 )
  {
    analogWrite(meterPin, 0);
    myservo.write(90);
    count = 0;
  }
    sensorValue = analogRead(sensorPin);
          if ( sensorValue > TRIGGERLEVEL )
  {
       digitalWrite(ledPin, HIGH); // LED on.
         milliSec = millis();
       timeDiff = milliSec - lastTime;
       lastTime = milliSec;
            if ( timeDiff < 1000 )
       {
             rpm = 100000 / timeDiff;
                val = map (rpm, 0, 500, 0, 255);
         if (val > 255) val = 255;
         analogWrite(meterPin, val);
                 Input = (double)rpm;
         myPID.Compute();
         if ( count > 18 )
         {
           myservo.write((int)Output);
           delay(15);
         }
         else
         {
           count = count + 1;
         }
                  Serial.print( "Time: " );
         Serial.print( timeDiff );
         Serial.print( "   RPM: " );
         Serial.print( rpm );
         Serial.print( "   Output: " );
         Serial.println( (int)Output );
                }
       delay(100);
       digitalWrite( ledPin, LOW ); // LED off.
   }
   }
