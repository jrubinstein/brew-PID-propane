// this is the code for HABS Automated Mode

void automode() 
{
  byte msgdisplayed=0;
  
  testmode  = EEPROM.read(0);
  countdowntimer = (10*EEPROM.read(1));
  checkfloat  = EEPROM.read(2);
  striketemp = EEPROM.read(3);
  targetmashtemp =  EEPROM.read(4);
  fudgefactor =  EEPROM.read(5);
  graintowaterratio = EEPROM.read(6);
  spargetemp = EEPROM.read(7); 
  doughinpumpruntime = EEPROM.read(8);
  mashlength = EEPROM.read(9);
  mashoutpumpruntime = EEPROM.read(10);
  mashoutstirtime = EEPROM.read(11);
  mashoutresttime = EEPROM.read(12);
  vorlaufpumpruntime = EEPROM.read(13);
  fillkettlepumpruntime = EEPROM.read(14);
  boiltemp = EEPROM.read(15);
  boillength = EEPROM.read(16);
  hopaddition1 = EEPROM.read(17);
  hopaddition2 = EEPROM.read(18);
  hopaddition3 = EEPROM.read(19);
  endofboilresttime = EEPROM.read(20);
  
  while (step<=totalsteps)       // this will stop the code from restarting from here on in!

  {  
    switch (step) 
    {
    case 1: // Step 1 Countdown Timer 
      if (!msgdisplayed)
      {
        Serial.print("?f 1. Time til Start");
        msgdisplayed=1;
      }
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)countdowntimer*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1) && (currenttemp1>0) && (currenttemp2>0)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        long countdowntimeremaining = (stopstep - stepelapsed) / 1000; // in seconds
        formattimeseconds(countdowntimeremaining,stepelapsedtime);
        Serial.print("?x03?y1");  
        Serial.print(stepelapsedtime);        
        cyclecount++;
      }
      break;  
    case 2: // step 2 - check float switch to see if there is water in the HLT
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

    case 3: // Step 3 Heating HLT to strike temp
      if (striketemp == 0)                                                                     
      {
        calcstrike();
      }
      if (!msgdisplayed)
      {
        Serial.print("?f 3. Heat HLT to ");   
        //Serial.print((int)striketemp);     // display striketemp                          
        Serial.print(striketemp);     // display calculated strike temp here                
        Serial.print("?0");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
        startstep=millis();
      }
      if (checkfloat == 1)
      {  
        if (!(floatswitch.isPressed()))
        {      
          digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
          Serial.print("?x12?y21");  // set element ssr to 1 (on)
      }
      }
      else
      {
        digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
        Serial.print("?x12?y21");  // set element ssr to 1 (on)
      }

      if ((currenttemp1>>4)>=striketemp) {                                                        
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
    case 4: // Step 4 Doughing-In
      if (!msgdisplayed)
      {
        striketime = stepelapsed; // this will store the value from the prevous step "heat hlt" of how long it took to get to striketemp
        Serial.print("?f 4. Doughing-In");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      digitalWrite(PinPumpHlt, HIGH);       // turn pump from hlt to mash tun on
      Serial.print("?x14?y21");  // set pump hlt ssr to 1 (on)
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)doughinpumpruntime*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        startmashmotor = startstep + (steptime/2); 
        timercalculated = 1; // don't do again whilst in this step
      }

      if (millis()>=startmashmotor && !startmashmotorreached) 
      { 
        digitalWrite(PinMotorMT, HIGH);       // turn mash tun motor on
        Serial.print("?x18?y21");  // set motor mtssr to 1 (on)
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
        formattime(0,doughinpumpruntime,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
    case 5: // Step 5 Stir Mash / Heat Sparge Water
      if (!msgdisplayed)
      {
        Serial.print("?f 5. Sacc Rest");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      digitalWrite(PinMotorMT, HIGH);       // turn mash tun motor on
      delay(100);
      Serial.print("?x18?y21");  // set motor mtssr to 1 (on)
      if ((currenttemp1byte>=spargetemp+1) && (currenttemp1byte>0))
      {
        digitalWrite(PinElementHlt, LOW);     // turn element off
        Serial.print("?x12?y20");  // set element ssr to 0 (off)
    }
      if (checkfloat == 1)
      {
        if (!(floatswitch.isPressed()))
        {
          if ((currenttemp1byte<=spargetemp-1) && (currenttemp1byte>0))
          {
            digitalWrite(PinElementHlt, HIGH);       // turn element on
            Serial.print("?x12?y21");  // set element ssr to 1 (on)
          }
        }
      } 
      else 
      {
        if ((currenttemp1byte<=spargetemp-1) && (currenttemp1byte>0))
        {
          digitalWrite(PinElementHlt, HIGH);       // turn element on
         Serial.print("?x12?y21");  // set element ssr to 1 (on)
      }
      } 
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)mashlength*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep5 = startstep + steptime5; //calc end time for 10 min mash temp recording
        stopstep10 = startstep + steptime10; //calc end time for 10 min mash temp recording
        stopstep30 = startstep + steptime30; //calc end time for 30 min mash temp recording
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep5 && !stopstep5reached) { // 10 min point reached to record mash temperature
        mashtemp5 = lasttemp2;   // grab the temperature now of the mash and store in memory to dislay later in end report
        stopstep5reached=true;
      }
      if (millis()>=stopstep10 && !stopstep10reached) { // 10 min point reached to record mash temperature
        mashtemp10 = lasttemp2;   // grab the temperature now of the mash and store in memory to dislay later in end report
        stopstep10reached=true;
      }
      if (millis()>=stopstep30 && !stopstep30reached) { // 30 min point reached to record mash temperature
        mashtemp30 = lasttemp2;   // grab the temperature now of the mash and store in memory to dislay later in end report
        stopstep30reached=true;
      }
      if (millis()>=stopstep) { // time has finished
        digitalWrite(PinElementHlt, LOW);    // turn pump off
        digitalWrite(PinMotorMT, LOW);       // turn mash tun motor off
        mashtempend = lasttemp2;   // grab the temperature now of the mash and store in memory to dislay later in end report
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,mashlength,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
    case 6: // Step 6 Mash Out 
      if (!msgdisplayed)
      {
        Serial.print("?f 6. Mash Out");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      digitalWrite(PinPumpHlt, HIGH);       // turn pump from hlt to mash tun on
      Serial.print("?x14?y21");  // set pump hlt ssr to 1 (on)
      digitalWrite(PinMotorMT, HIGH);       // turn mash tun motor on
      Serial.print("?x18?y21");  // set motor mtssr to 1 (on)
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)mashoutpumpruntime*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        digitalWrite(PinPumpHlt, LOW);    // turn pump off
        digitalWrite(PinMotorMT, LOW);       // turn mash tun motor off
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,mashoutpumpruntime,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
          case 7: // Step 7 Stir after adding Mash Out water  
      if (!msgdisplayed)
      {
        Serial.print("?f 7. Stir/Mash Out");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      digitalWrite(PinMotorMT, HIGH);       // turn mash tun motor on
      Serial.print("?x18?y21");  // set motor mtssr to 1 (on)
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)mashoutstirtime*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        digitalWrite(PinMotorMT, LOW);       // turn mash tun motor off
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,mashoutstirtime,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
    case 8: // Step 8 Pause after Mash Out 
      if (!msgdisplayed)
      {
        Serial.print("?f 8. Mash Out Rest");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)mashoutresttime*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        //move to next step after "spargeresttime" has passes
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,mashoutresttime,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
    case 9: // Step 9 Vorlauf            
      if (!msgdisplayed)
      {
        Serial.print("?f 9. Vorlauf");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      digitalWrite(PinPumpMT, HIGH);       // turn pump from mash tun to hlt/kettle on
      Serial.print("?x16?y21");  // set pump mt ssr to 1 (on)
      digitalWrite(PinPumpHlt, HIGH);       // turn pump from hlt to mashtun on
      Serial.print("?x14?y21");  // set pump hlt ssr to 1 (on)
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)vorlaufpumpruntime*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        //digitalWrite(PinPumpMT, LOW);       // turn pump from mash tun to hlt/kettle on
        digitalWrite(PinPumpHlt, LOW);    // turn pump off
        mashouttemp = lasttemp2;   // grab the temperature now of the mash and store in memory to dislay later in end report
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,vorlaufpumpruntime,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;
    case 10: // Step 10 Fill Kettle
      if (!msgdisplayed)
      {
        Serial.print("?f10. Fill Kettle");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      digitalWrite(PinPumpMT, HIGH);       // turn pump from hlt to mashtun on
      delay(50);
      Serial.print("?x16?y21");  // set pump mt ssr to 1 (on)
      if (checkfloat == 1)
      {
        if (!(floatswitch.isPressed()))
          {
            digitalWrite(PinElementHlt, HIGH);       // turn element on
            delay(50);
            Serial.print("?x12?y21");  // set element ssr to 1 (on)
            delay(50);  
        }
      } 
      else 
        {
          digitalWrite(PinElementHlt, HIGH);       // turn element on
          Serial.print("?x12?y21");  // set element ssr to 1 (on)
        }
 
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)fillkettlepumpruntime*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
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
        formattime(0,fillkettlepumpruntime,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;        
    case 11: // Step 11 Bring Kettle to Boil
      if (!msgdisplayed)
      {
        Serial.print("?f11. Bring to Boil ");
        delay(50);  
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50);   
        msgdisplayed=1;
        startstep=millis();
      }
      if (checkfloat == 1)
      {  
        if (!(floatswitch.isPressed()))
        {      
          digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
          Serial.print("?x12?y21");  // set element ssr to 1 (on)
        }
      }
        else
        {
          digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
          Serial.print("?x12?y21");  // set element ssr to 1 (on)
        }
      
      if ((currenttemp1>>4)>=boiltemp) {
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

    case 12: // Step 12 Boil Kettle
      if (!msgdisplayed)
      {
        boiltime = stepelapsed; // this will store the value from the previous step "bring to boil" of how long it took to get to boil
        Serial.print("?f12. Boil Kettle");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      if (checkfloat == 1)
      {  
        if (!(floatswitch.isPressed()))
        {      
          digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
          Serial.print("?x12?y21");  // set element ssr to 1 (on)
        }
      }
        else
        {
          digitalWrite(PinElementHlt, HIGH);       // turn element on from hlt/kettle
          Serial.print("?x12?y21");  // set element ssr to 1 (on)
      }
      
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)boillength*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        hopadditiontime1 = (unsigned long)hopaddition1*60*1000;
        hopadditionbeepstart1 = (steptime - hopadditiontime1)+startstep;  // this calculates when the hop addition is needed to beep/add hops
        hopadditionbeepstop1 = (unsigned long)hopadditionbeepstart1+5000;  // this calculates when the hop addition is needed to beep/add hops
        hopadditiontime2 = (unsigned long)hopaddition2*60*1000;
        hopadditionbeepstart2 = (steptime - hopadditiontime2)+startstep;  // this calculates when the hop addition is needed to beep/add hops
        hopadditionbeepstop2 = (unsigned long)hopadditionbeepstart2+5000;  // this calculates when the hop addition is needed to beep/add hops
        hopadditiontime3 = (unsigned long)hopaddition3*60*1000;
        hopadditionbeepstart3 = (steptime - hopadditiontime3)+startstep;  // this calculates when the hop addition is needed to beep/add hops
        hopadditionbeepstop3 = (unsigned long)hopadditionbeepstart3+5000;  // this calculates when the hop addition is needed to beep/add hops
        timercalculated = 1; // don't do again whilst in this step
      }
      //  beeping for desired hop addition times during the boil (will be automated hop additions down the track)
      if ((millis()>=hopadditionbeepstart1) && (millis()<=hopadditionbeepstop1))
      {
        beep(PinSpeaker,2093,500); 	//C: play the note C (C7 from the chart linked to above) for 500ms
        Serial.print("?x01?y0Add ");
        Serial.print((int)hopaddition1);
        Serial.print(" Minute Hops ");  
        Serial.print("?g");                //this will beep the remote speaker thru Screen Emulator Program on PC
      }
      if ((millis()>=hopadditionbeepstart2) && (millis()<=hopadditionbeepstop2))
      {
        beep(PinSpeaker,2093,500); 	//C: play the note C (C7 from the chart linked to above) for 500ms  
        Serial.print("?x01?y0Add ");
        Serial.print((int)hopaddition2);
        Serial.print(" Minute Hops ");  
        Serial.print("?g");                //this will beep the remote speaker thru Screen Emulator Program on PC        
      }
      if ((millis()>=hopadditionbeepstart3) && (millis()<=hopadditionbeepstop3))
      {
        beep(PinSpeaker,2093,500); 	//C: play the note C (C7 from the chart linked to above) for 500ms          
        Serial.print("?x01?y0Add ");
        Serial.print((int)hopaddition3);
        Serial.print(" Minute Hops ");  
        Serial.print("?g");                //this will beep the remote speaker thru Screen Emulator Program on PC  
      }
      // finish hop addition code 
      if (millis()>=stopstep) { // time has finished
        digitalWrite(PinElementHlt, LOW);
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,boillength,0,steptotaltime);
        Serial.print("?x00?y1stp");              
        Serial.print(stepelapsedtime);
        Serial.print("/");
        Serial.print(steptotaltime);
        cyclecount++;
      }
      break;

case 13: // Step 13 End of boil rest  
      if (!msgdisplayed)
      {
        Serial.print("?f14. End Boil Rest");
        delay(50); 
        Serial.print("?x12?y20,0,0,0");  // initially set all values to 0 (off)
        delay(50); 
        msgdisplayed=1;
      }
      if (!timercalculated) {
        startstep = millis(); // start step clock, reads milliseconds from this moment
        steptime = (unsigned long)endofboilresttime*60*1000; // calculate the end of step from the configuired time "hltpumpruntime"
        stopstep = startstep + steptime; // calculate the end of step from the configuired time "hltpumpruntime"
        timercalculated = 1; // don't do again whilst in this step
      }
      if (millis()>=stopstep) { // time has finished
        step++;
        msgdisplayed=0;
        timercalculated=0; // next step's timer not yet calculated
      }
      if (!(cyclecount % 1)) { //only update display each 1000th time thru loop  //TODO increase value when process is running fast (ie checking time properly)
        stepelapsed = (long)(millis()-startstep); // caculate how many milliseconds since step start time
        stepelapsedsecs = (int)(stepelapsed/1000); // convert into human readable second like intervals but not exact
        formattimeseconds(stepelapsedsecs,stepelapsedtime);
        formattime(0,endofboilresttime,0,steptotaltime);
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

  Serial.print("?f@5=");
  delay(50);
  Serial.print(mashtemp5>>4);
  delay(50);
  overflow = mashtemp5 % 16;
  decimalvalue = temparray[overflow]+48;
  Serial.print("."); 
  delay(50);
  Serial.print(decimalvalue); 
  delay(50);
  Serial.print("?0"); //deg C
  delay(50);
  
  Serial.print("? @10=");
  delay(50);
  Serial.print(mashtemp10>>4);
  delay(50);
  overflow = mashtemp10 % 16;
  decimalvalue = temparray[overflow]+48;
  Serial.print(".");
  delay(50); 
  Serial.print(decimalvalue);
  delay(50); 
  Serial.print("?0"); //deg C
  delay(50);
  
  Serial.print("?n@30=");
  delay(50);
  Serial.print(mashtemp30>>4);
  delay(50);
  overflow = mashtemp30 % 16;
  decimalvalue = temparray[overflow]+48;
  Serial.print(".");
  delay(50); 
  Serial.print(decimalvalue);
  delay(50); 
  Serial.print("?0"); //deg C
  delay(50);

  Serial.print(" END=");
  delay(50);
  Serial.print(mashtempend>>4);
  delay(50);
  overflow = mashtempend % 16;
  decimalvalue = temparray[overflow]+48;
  Serial.print(".");
  delay(50); 
  Serial.print(decimalvalue); 
  delay(50);
  Serial.print("?0"); //deg C
  delay(50);

  Serial.print("?nOUT=");
  delay(50);
  Serial.print(mashouttemp>>4);
  delay(50);
  overflow = mashouttemp % 16;
  decimalvalue = temparray[overflow]+48;
  Serial.print(".");
  delay(50); 
  Serial.print(decimalvalue); 
  delay(50);
  Serial.print("?0"); //deg C
  delay(50);
  
  Serial.print("?x15?y3NEXT");   // displays soft button next 

    while (1)   //now loop for ever to stop program ending
  {
    if (Serial.available() > 0)
    {
      incoming = Serial.read();
    }
    else
    {
      incoming = 0;
    }

    if ((button4.uniquePress()) || ((char)incoming == '4'))
    {
      break;
    }
  }
  
  Serial.print("?fSTRIKE=");
  long striketemptime=0;
  striketemptime=(long)striketime/1000;
  formattimeseconds(striketemptime,temptime);
  Serial.print(temptime);
  delay(100);

  Serial.print("?nBOIL=");
  long boiltemptime=0;
  boiltemptime=(long)boiltime/1000;
  formattimeseconds(boiltemptime,temptime);
  Serial.print(temptime);
  delay(100);

  Serial.print("?nTOTAL=");
  Serial.print(totaltime);
  delay(100);
  
  while (1) // do nothing
  {
  }
}







