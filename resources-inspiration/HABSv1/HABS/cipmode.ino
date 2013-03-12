// this is the code for HABS CIP (Cleaning In Place) Mode

void cipmode() 
{
  byte msgdisplayed=0;

  testmode  = EEPROM.read(0);
  checkfloat  = EEPROM.read(2);
  
  while (step<=5)       // this will stop the code from restarting from here on in!

  {  
    switch (step) 
    {
    case 1: // step 1 - check float switch to see if there is water in the HLT
      if (checkfloat == 1)
      {  
        if ((!msgdisplayed) && (floatswitch.isPressed()))  
        {
          Serial.print("?fPlease fill HLT?n");        //displays a question and waits for button to be pressed before continuing
          Serial.print("/check float switch");
          msgdisplayed=1;
        }
        if (!(floatswitch.isPressed()))       
        {
          step++;  // go to step 2
          msgdisplayed=0;
        }
      }
      else
      {
        step++;  // go to step 2
        msgdisplayed=0;
      }
      break;      

    case 2: // Step 2 Heating HLT to strike temp
      if (!msgdisplayed)
      {
        Serial.print("?f2. Heat HLT to ");   
        Serial.print("75");     // display striketemp number here
        Serial.print("?0"); 
        msgdisplayed=1;
        startstep=millis();
      }
      if (checkfloat == 1)
      {  
        if (!(floatswitch.isPressed()))
        {      
          digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
        }
      }
      else
      {
        digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
      }

      if ((currenttemp1>>4)>=75) {
        digitalWrite(PinElementHlt, LOW);       // turn element off
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        cyclecount++;
      }
      break;
    case 3: // Step 3 Doughing-In
      if (!msgdisplayed)
      {
        striketime = stepelapsed; // this will store the value from the prevous step "heat hlt" of how long it took to get to striketemp
        Serial.print("?f3. xfer 1/2 to MT");
        msgdisplayed=1;
      }
      digitalWrite(PinPumpHlt, HIGH);       // turn pump from hlt to mash tun on
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)11*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        startmashmotor = startstep + (steptime/2); 
        timercalculated = 1; // don't do again whilst in this step
      }

      if (millis()>=startmashmotor && !startmashmotorreached) 
      { 
        digitalWrite(PinMotorMT, HIGH);       // turn mash tun motor on
        startmashmotorreached=true;
      }

      if (millis()>=stopstep) { // time has finished
        digitalWrite(PinPumpHlt, LOW);    // turn pump off
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,11,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
    case 4: // Step 4 Stir Mash / Heat Sparge Water
      if (!msgdisplayed)
      {
        Serial.print("?f4. Stir/Recirc/Heat");
        msgdisplayed=1;
      }
      digitalWrite(PinMotorMT, HIGH);       // turn mash tun motor on
      digitalWrite(PinPumpMT, HIGH);       // turn pump from mash tun to hlt/kettle on
      digitalWrite(PinPumpHlt, HIGH);       // turn pump from hlt to mashtun on
      
	  // when variable "spargetemphigh" gets to desired temp switch the heating element off
      if ((currenttemp1byte>=75) && (currenttemp1byte>0))
      {
        digitalWrite(PinElementHlt, LOW);     // turn element off
      
		}
      // when variable "spargetemplow" gets below a desired temp switch the heating element on
      if (checkfloat == 1)
      {
        if (!(floatswitch.isPressed()))
        {
          if ((currenttemp1byte<=70) && (currenttemp1byte>0))
          {
            digitalWrite(PinElementHlt, HIGH);       // turn element on
          }
        }
      } 
      else 
      {
        if ((currenttemp1byte<=70) && (currenttemp1byte>0))
        {
          digitalWrite(PinElementHlt, HIGH);       // turn element on
        }
      } 
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)30*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        digitalWrite(PinElementHlt, LOW);    // turn pump off
        digitalWrite(PinMotorMT, LOW);       // turn mash tun motor off
        digitalWrite(PinPumpMT, LOW);       // turn pump from mash tun to hlt/kettle on
        digitalWrite(PinPumpHlt, LOW);       // turn pump from hlt to mashtun on
      
	 step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,30,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
    case 5: // Step 5 Fill Kettle
      if (!msgdisplayed)
      {
        Serial.print("?f5. Fill Kettle");
        msgdisplayed=1;
      }
      digitalWrite(PinPumpMT, HIGH);       // turn pump from hlt to mashtun on

       if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)16*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        digitalWrite(PinPumpMT, LOW);    // turn pump off
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,16,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;        
    }

    if ((button2.uniquePress()) || ((char)incoming == '2'))
    {
      if (testmode == 1)  /// in testmode this will allow me to skip to the next step
      {
        digitalWrite(PinElementHlt, LOW);
        digitalWrite(PinPumpHlt, LOW);
        digitalWrite(PinPumpMT, LOW);
        digitalWrite(PinMotorMT, LOW);
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
    }  

    Core();  // temperature check / total time

  } // end main loop

  byte overflow;
  byte decimalvalue;
                    
  Serial.print("?fHEAT HLT=");
  long striketemptime=0;
  striketemptime=(long)striketime/1000;
  formattimeseconds(striketemptime,temptime);
  Serial.print(temptime);

  Serial.print("?nTOTAL=");
  Serial.print(totaltime);

  while (1)   //now loop for ever to stop program ending
  {
  }
}







