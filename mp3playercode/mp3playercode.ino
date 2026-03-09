/*
Developers: Will Holmes, Cyle Krohling, and Kiera McKimmy

resources: https://www.oceanlabz.in/getting-started-with-serial-mp3-player/
https://forum.arduino.cc/t/using-serialmp3player-library-with-yx5300-mp3-player-and-arduino-mega/1194648
http://github.com/salvadorrueda/SerialMP3Player/tree/master

LCD Troubleshooting Code:https://chatgpt.com/share/69a5dca7-a304-8002-a3b0-b455103b4850
B10K: https://chatgpt.com/share/69a5e2ae-5de0-8002-92cb-9df7dc2da5c6
Buttons: https://claude.ai/share/514c4088-8de8-4913-9fd6-031d9d5ecccd

*/


/******************************************************************************
  Basic Commands examples for the SerialMP3Player YX5300 chip.

  Copy the files of "SDcard_example" to an empty SD card
  Connect the Serial MP3 Player to the Arduino board
    GND → GND
    VCC → 5V
    TX → pin 11
    RX → pin 10

  After compile and upload the code,
  you can test some basic commands by sending the letters
  ? - Display Menu options.
  P01 - Play 01 file
  F01 - Play 01 folder
  S01 - Play 01 file in loop
  p - play
  a - pause
  s - stop
  > - Next
  < - Previous
  ...

  Some commands like 'P' must be followed by two digits.

  This example code is in the public domain.

  https://github.com/salvadorrueda/ArduinoSerialMP3Player

  by Salvador Rueda
 *******************************************************************************/

#include "SerialMP3Player.h" 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TX 3
#define RX 2
#define POT A0

// --- Button pins ---
#define BTN_NEXT       11
#define BTN_PREV       9
#define BTN_PLAYPAUSE  10

// Upload this alone to test your buttons
void setup() {
  Serial.begin(9600);
  pinMode(11, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(11) == LOW) Serial.println("NEXT pressed");
  if (digitalRead(9) == LOW) Serial.println("PREV pressed");
  if (digitalRead(109) == LOW) Serial.println("PLAY pressed");
  delay(100);
}