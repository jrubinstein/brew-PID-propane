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
 * Components needed:
 * - Atmega 328 (microcontroller)
 * - Solid State Relay
 * - DS18B20 based temperature sensor
 * - HD77480 LCD panel (20 x 4)
 * - 3 push buttons
 * - 16 mhz resonator or crystal
 * - A few basic resistors, caps, leds and transistors.
 *
 * Pins-layout:
 * - LCD: 11, 12, A0, A1, A2, A3
 * - Onewire: 5
 * - SSR: 10
 * - Buttons: 8 (up), 7 (down), 6 (set)
 * - Status LED: 9
 *
 */