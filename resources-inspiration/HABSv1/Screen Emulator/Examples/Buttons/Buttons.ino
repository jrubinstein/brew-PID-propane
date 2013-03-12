/*

 Screen Emulator Buttons Example Sketch - 29/5/09
 Demonstrates the uses of "Screen Emulator" PC Program
 
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

byte ReadPin13;       //placeholder for pin state  

void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop()
{

  ReadPin13 = digitalRead(13);    // This will check the state of Pin 13

    if (Serial.available() > 0)
  {
    int incoming = Serial.read();

    if ((char)incoming == '1')
    {
      if (ReadPin13 == LOW)
      {
        digitalWrite(13, HIGH);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON ONE PRESSED?n");
        Serial.print("?n");
        Serial.print("LED ON PIN 13 HIGH");
        Serial.print("?g");               // Beep 
      }
      else
      {
        digitalWrite(13, LOW);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON ONE PRESSED?n");
        Serial.print("?n");
        Serial.print("LED ON PIN 13 LOW");
        Serial.print("?g");               // Beep 
      }
    } 
    if ((char)incoming == '2')
    {
      if (ReadPin13 == LOW)
      {
        digitalWrite(13, HIGH);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON TWO PRESSED?n");
        Serial.print("?n");
        Serial.print("LED ON PIN 13 HIGH");
        Serial.print("?g");               // Beep 
      }
      else
      {
        digitalWrite(13, LOW);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON TWO PRESSED?n");
        Serial.print("?n");
        Serial.print("LED ON PIN 13 LOW");
        Serial.print("?g");               // Beep 
      }
    } 
    if ((char)incoming == '3')
    {
      if (ReadPin13 == LOW)
      {
        digitalWrite(13, HIGH);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON THREE PRESSED?n");
        Serial.print("LED ON PIN 13 HIGH");
        Serial.print("?g");               // Beep 
      }
      else
      {
        digitalWrite(13, LOW);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON THREE PRESSED?n");
        Serial.print("LED ON PIN 13 LOW");
        Serial.print("?g");               // Beep 
      }
    } 
    if ((char)incoming == '4')
    {
      if (ReadPin13 == LOW)
      {
        digitalWrite(13, HIGH);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON FOUR PRESSED?n");
        Serial.print("?n");
        Serial.print("LED ON PIN 13 HIGH");
        Serial.print("?g");               // Beep 
      }
      else
      {
        digitalWrite(13, LOW);
        Serial.print("?f");               // Clear Screen
        Serial.print("BUTTON FOUR PRESSED?n");
        Serial.print("?n");
        Serial.print("LED ON PIN 13 LOW");
        Serial.print("?g");               // Beep 
      }
    } 
  }
}

