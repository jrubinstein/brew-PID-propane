// constanty read and display temps, elapsed time and allow for on/off operation of SSR's

void manualmode()  

{
  Serial.print("?f");
  delay(100);
  Serial.print("?x00?y0 HE   PHLT PMT  MM"); 
  delay(100);
  Serial.print("?x00?y1 OFF  OFF  OFF  OFF"); 
  delay(100);
  
  checkfloat  = EEPROM.read(2);

  while (1) 
  {

   Core();  // temperature check / total time

    {
      ReadPinElementHlt = digitalRead(PinElementHlt); 
      ReadPinPumpHlt = digitalRead(PinPumpHlt);
      ReadPinPumpMT = digitalRead(PinPumpMT);
      ReadPinMotorMT = digitalRead(PinMotorMT);

      // for  hlt element ssr, also checks to see if float switch high before turning element on 

      if (((button1.uniquePress()) || ((char)incoming == '1')))
      { 
        if (checkfloat == 1)
        {
          if (!(floatswitch.isPressed()))
          {
            if ((ReadPinElementHlt == LOW))
            {
              digitalWrite(PinElementHlt, HIGH);
              Serial.print("?x01?y1ON ");
            }
            else
            {
              digitalWrite(PinElementHlt, LOW);
              Serial.print("?x01?y1OFF");
            }
          }
        }
        else 
        {
          if ((ReadPinElementHlt == LOW))
          {
            digitalWrite(PinElementHlt, HIGH);
            Serial.print("?x01?y1ON ");
          }
          else
          {
            digitalWrite(PinElementHlt, LOW);
            Serial.print("?x01?y1OFF");
          }
        }
      }


      //      for hlt pump ssr

      if (((button2.uniquePress()) || ((char)incoming == '2')))
      {
        if (ReadPinPumpHlt == LOW)
        {
          digitalWrite(PinPumpHlt, HIGH);
          Serial.print("?x06?y1ON ");
        }
        else
        {
          digitalWrite(PinPumpHlt, LOW);
          Serial.print("?x06?y1OFF");
        }
      }

      //     for mashtun pump ssr

      if (((button3.uniquePress()) || ((char)incoming == '3')))
      {
        if (ReadPinPumpMT == LOW)
        {
          digitalWrite(PinPumpMT, HIGH);
          Serial.print("?x11?y1ON ");
        }
        else
        {
          digitalWrite(PinPumpMT, LOW);
          Serial.print("?x11?y1OFF");
        }
      }

      //      for mash stirrer ssr

      if (((button4.uniquePress()) || ((char)incoming == '4')))
      {
        if (ReadPinMotorMT == LOW)
        {
          digitalWrite(PinMotorMT, HIGH);
          Serial.print("?x16?y1ON ");
        }
        else
        {
          digitalWrite(PinMotorMT, LOW);
          Serial.print("?x16?y1OFF");
        }
      }
    }
  }
}



