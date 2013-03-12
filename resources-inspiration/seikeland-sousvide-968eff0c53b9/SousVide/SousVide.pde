/*
 * Sous Vide controller by Stian Eikeland
 * <stian.eikeland@gmail.com>
 *
 * Libraries used:
 * - Button: https://github.com/tigoe/Button
 * - OneWire: http://www.pjrc.com/teensy/td_libs_OneWire.html
 * - DallasTemperature: http://www.milesburton.com/?title=Dallas_Temperature_Control_Library
 * - PID v1: http://code.google.com/p/arduino-pid-library/
 * - LiquidCrystal: http://www.arduino.cc/en/Tutorial/LiquidCrystal
 *
 * Video demonstration at:
 * http://vimeo.com/26730692
 *
 * Please feel free to use the code for whatever you want,
 * would love to hear about it if you do :)
 *
 * Pins-layout:
 * - LCD: 11, 12, A0, A1, A2, A3
 * - Onewire: 5
 * - SSR: 10
 * - Buttons: 8 (up), 7 (down), 6 (set)
 * - Status LED: 9
 *
 */

#define DEBUGMODE

#ifndef DEBUGMODE
#define DEBUG(a)
#define DEBUGSTART(a)
#else
#define DEBUG(a) Serial.println(a);
#define DEBUGSTART(a) Serial.begin(a);
#endif

#include <Button.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

#define LCDSIZE 20, 4

#define SSR 10
#define ONEWIRE 5
#define BTN_UP 8
#define BTN_DOWN 7
#define BTN_SET 6
#define LED 9
#define LCD 11, 12, A0, A1, A2, A3

enum programmode {
	menumode,
	cooking,
	startup,
	probedetected,
	noprobe
};

programmode mode = startup;

LiquidCrystal lcd(LCD);

#define RESOLUTION 12
#define TEMPINTERVAL 2000

OneWire oneWire(ONEWIRE);
DallasTemperature sensor(&oneWire);
DeviceAddress tempDeviceAddress;

// If you get compilation errors here, you might have a
// different version of the Button library, try
// changing PULLUP to BUTTON_PULLUP
Button btnUp = Button(BTN_UP, PULLUP);
Button btnDown = Button(BTN_DOWN, PULLUP);
Button btnSet = Button(BTN_SET, PULLUP);

unsigned long lastTempRequest = 0;
unsigned long lastPIDCalculation = 0;
float temperature;
float prevTemperature = -9999.0;

double pidSetPoint = 60;
double pidInput, pidOutput;

//These values needs to be tuned to your specific cooker:
PID pid(&pidInput, &pidOutput, &pidSetPoint, 1.8, 0.3, 200, DIRECT);
//PID pid(&pidInput, &pidOutput, &pidSetPoint, 2, 5, 1, DIRECT);
//PID pid(&pidInput, &pidOutput, &pidSetPoint, 200, 90, 300, DIRECT);


void setup()
{
	DEBUGSTART(9600);
	DEBUG("Running setup..");
	
	lcd.begin(LCDSIZE);
	printScreen();
	
	pinMode(SSR, OUTPUT);
	pinMode(LED, OUTPUT);
	
	sensor.begin();
	
	// Sensor detected?
	checkSensor();
	
	pid.SetOutputLimits(0, TEMPINTERVAL);
	pid.SetSampleTime(TEMPINTERVAL);
	
	delay(2000);
	mode = menumode;
	printScreen();
}

/* Detect sensor, ask user to attach if not.. */
void checkSensor()
{
	DEBUG("Checking sensor..");
	sensor.begin();
	
	// Wait for sensor and ask user
	while (sensor.getDeviceCount() == 0)
	{
		mode = noprobe;
		printScreen();
	
		DEBUG("No sensor detected..");
		delay(1000);
		sensor.begin();
	}
	
	mode = probedetected;
	printScreen();
	
	// Set sensor options and request first sample
	sensor.getAddress(tempDeviceAddress, 0);
	sensor.setResolution(tempDeviceAddress, RESOLUTION);
	sensor.setWaitForConversion(false);
	sensor.requestTemperatures();
	lastTempRequest = millis();
}

void menu()
{
	bool changed = false;
	
	if (btnUp.uniquePress()) {
		DEBUG("Button Up");
		pidSetPoint += 1.0;
		changed = true;
	}
	
	if (btnDown.uniquePress()) {
		DEBUG("Button Down");
		pidSetPoint -= 1.0;
		changed = true;
	}
	
	if (changed) {
		printScreen();
	}
	
	delay(10);
}

void printScreen()
{
	switch(mode) {
		
		case startup:
			lcd.clear();
			lcd.noCursor();
		case cooking:
			lcd.setCursor(0,0); 
			lcd.print("  SousVide-o-Mator  ");
			break;
			
		case menumode:
			lcd.setCursor(0,0);
			lcd.print("  ** Setup mode **  ");
			lcd.setCursor(0,3);
			lcd.print(" Target temp: ");
			lcd.print(int(pidSetPoint));
			lcd.print(".0 C");
			break;
		
		case noprobe:
			lcd.setCursor(0, 2);
			lcd.print("Sensor not detected,");
			lcd.setCursor(0, 3);
			lcd.print("please attach probe.");
			break;
			
		case probedetected:
			lcd.setCursor(0, 2);
			lcd.print("Temp-Sensor detected");
			lcd.setCursor(0, 3);
			lcd.print("                    ");
			break;
			
	}
}

void cook()
{
	
	if ((millis() <= (lastPIDCalculation + pidOutput)) || (pidOutput == TEMPINTERVAL)) {
		// Power cooker on:
		digitalWrite(SSR, HIGH);
		digitalWrite(LED, HIGH);
	} else {
		// Power cooker off:
		digitalWrite(SSR, LOW);
		digitalWrite(LED, LOW);
	}
	
	delay(10);
}

void loop()
{
	// Check temperature and request new sample:
	if (millis() - lastTempRequest >= TEMPINTERVAL) {
		temperature = sensor.getTempCByIndex(0);
		pidInput = (double)temperature;

		if (temperature <= -100) {
  			// Probe disconnected?
			temperature = prevTemperature;
			pidInput = (double)temperature;
			
			// Turn off cooker:
			digitalWrite(SSR, LOW);
			digitalWrite(LED, LOW);
			
			// Scan for probe, and enter setup
			// once probe is attached again.
			checkSensor();
			mode = menumode;
			printScreen();
  		}
		
		// Calculate PID value:
		if (mode == cooking) {
			pid.Compute();
			lastPIDCalculation = millis();
		}
		
		if (temperature != prevTemperature) {
			lcd.setCursor(0, 2);
			lcd.print("Current temp: ");
			lcd.print(temperature, 1);
			lcd.print(" C");
			
			DEBUG("Temperature:");
			DEBUG(temperature);
			DEBUG("Output:");
			DEBUG(pidOutput);
		}
		
		prevTemperature = temperature;
		
		sensor.requestTemperatures();
		lastTempRequest = millis();
	}
		
	// Change mode?
	if (btnSet.uniquePress()) {
		if (mode == menumode) {
			pid.SetMode(AUTOMATIC);
			DEBUG("Cooking mode..");
			mode = cooking;
			printScreen();
			
		} else if (mode == cooking) {
			pid.SetMode(MANUAL);
			digitalWrite(SSR, LOW);
			digitalWrite(LED, LOW);
			DEBUG("Setup mode..");
			mode = menumode;
			printScreen();
		}
	}
	
	if (mode == menumode)
		menu();
	else if (mode == cooking)
		cook();
	
}
