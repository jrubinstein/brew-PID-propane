// Setup Menu for user defined variables 
//
// ATmega328P has 1024 bytes of EEPROM, values are from 0-1023
// 
//  0 = testmode   (DEFAULT 0, SET TO 1 TO ENABLE SKIP STEP) When testmode is enabled you can skip thru steps in AutoMode by pressing button 2
//  1 = countdowntimer (DEFAULT 0, SET BETWEEN 0-255 FOR DELAY AT START OF AUTOMODE)
//  2 = checkfloat (DEFAULT 1, SET TO 0 IF YOU DON'T REQUIRE CHECKING OF THE FLOAT SWITCH)
//  3 = striketemp  (DEFAULT 0, OTHERWISE MANUALLY SET)
//  4 = targetmashtemp (DEFAULT 63-69) 
//  5 = fudgefactor (DEFAULT 5)
//  6 = graintowaterratio (DEFAULT 3)   
//  7 = spargetemp (DEFAULT 98)
//  8 = doughinpumpruntime (DEFAULT 9) ***
//  9 = mashlength (DEFAULT 60)
// 10 = mashoutpumpruntime (DEFAULT 6) ***
// 11 = mashoutstirtime (DEFAULT 5)
// 12 = mashoutresttime (DEFAULT 10)
// 13 = vorlaufpumpruntime (DEFAULT 10) ***
// 14 = fillkettlepumpruntime (DEFAULT 11) ***
// 15 = boiltemp (DEFAULT 99)
// 16 = boillength (DEFAULT 90)
// 17 = hopaddition1 (DEFAULT 60)
// 18 = hopaddition2 (DEFAULT 15)
// 19 = hopaddition3 (DEFAULT 5)
// 20 = endofboilresttime (DEFAULT 5)


void setupmenu()
{
const char* setupmenutext[]={"Test Mode          ",
                             "Countdown Timer    ",
                             "Check Float        ",
                             "Strike Temp        ",
                             "Target Mash Temp   ",
                             "Fudge Factor       ",
                             "Grain 2 Water Ratio",
                             "Sparge Water Temp  ",
                             "Doughin Pump Time  ",
                             "Mash Length        ",
                             "Mashout Pump Time  ",
                             "Mashout Stir Time  ",
                             "Mashout Rest Time  ",
                             "Vorlauf Pump Time  ",
                             "FillKettle Pmp Time",
                             "Boil Temp          ",
                             "Boil Length        ",
                             "Hop Addition 1     ",
                             "Hop Addition 2     ",
                             "Hop Addition 3     ",
                             "EndofBoil Rest Time"}; 
                        
  byte saveflagchanged = false;
  byte empromvalue;
  byte empromlocation = 0;
  empromvalue = EEPROM.read(empromlocation);
  delay(100);
  Serial.print("?f");
  Serial.print(setupmenutext[empromlocation]);  // print some text relavent here to correspond with the location number being changed
  delay(100);
  Serial.print("?x00?y1");      // put cursor here
  delay(100);
  Serial.print(empromvalue, DEC);
  delay(100);
  Serial.print("?x00?y3UP  DOWN  SAVE  NEXT");
  //            12345678901234567890
  delay (500);

  while(1)
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
      empromvalue++;        // increment value
      Serial.print("?x00?y1...");   //clear 3 digits space 
      delay(100);
      Serial.print("?x00?y1");      // put cursor here
      delay(100);
      Serial.print(empromvalue, DEC); // print value on screen
      delay(100);
      saveflagchanged=true;
    }
    if ((button2.uniquePress()) || ((char)incoming == '2'))
    {
      empromvalue--;        // increment value
      Serial.print("?x00?y1...");
      delay(100);
      Serial.print("?x00?y1");
      delay(100);
      Serial.print(empromvalue, DEC);
      delay(100);
      saveflagchanged=true;
    }
    if ((button3.uniquePress()) || ((char)incoming == '3'))
    {
      EEPROM.write(empromlocation, empromvalue);   // this writes the value "empromvalue" into "empromlocation" 
    }
    if ((button4.uniquePress()) || ((char)incoming == '4'))
    {
      empromlocation++; // advance to the next address of the EEPROM
      Serial.print("?x00?y0");
      Serial.print(setupmenutext[empromlocation]);  // print some text relavent here to correspond with the location number being changed
      delay(100);
      empromvalue = EEPROM.read(empromlocation);
      delay(100);
      Serial.print("?x00?y1...");
      delay(100);
      Serial.print("?x00?y1");
      delay(100);
      Serial.print(empromvalue, DEC);
      if (empromlocation == 21)   // once it gets to eeprom location 22 then return back to main menu
      return; 
    }
    if (saveflagchanged=true)
    {
      if (empromvalue != EEPROM.read(empromlocation))
      {
        Serial.print("?x14?y3*");
        delay(100);
      }
      else
      {
        Serial.print("?x14?y3 ");
        delay(100);
      }
      saveflagchanged=false; //don't update display until value changes again
    }
  }
}



