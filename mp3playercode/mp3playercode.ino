/*
Developers: Will Holmes, Cyle Krohling

resources: https://www.oceanlabz.in/getting-started-with-serial-mp3-player/
https://forum.arduino.cc/t/using-serialmp3player-library-with-yx5300-mp3-player-and-arduino-mega/1194648
http://github.com/salvadorrueda/SerialMP3Player/tree/master




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

SerialMP3Player mp3(RX, TX);
LiquidCrystal_I2C lcd(0x27, 16, 2);

char c;
char cmd=' ';
char cmd1=' ';

void setup() {
  Serial.begin(9600);
  mp3.begin(9600);
  delay(500);

  mp3.sendCommand(CMD_SEL_DEV, 0, 2); // select SD
  delay(500);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("MP3 Ready");

  menu('?',0);
}

void loop() {

  // Serial menu control
  if (Serial.available()){
    c = Serial.read();
    decode_c();
  }

  // Check for MP3 answers
  if (mp3.available()){
    String answer = mp3.decodeMP3Answer();
    Serial.println(answer);

    // If response contains track number, extract it
    if (answer.indexOf("Playing") >= 0) {
      int track = answer.substring(answer.lastIndexOf(" ")+1).toInt();

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Track:");
      lcd.setCursor(7,0);
      lcd.print(track);
    }
  }
}

void menu(char op, int nval){

  switch (op){

    case 'P': mp3.play(nval); break;
    case 'F': mp3.playF(nval); break;
    case 'S': mp3.playSL(nval); break;
    case 'p': mp3.play(); break;
    case 'a': mp3.pause(); break;
    case 's': mp3.stop(); break;
    case '>': mp3.playNext(); break;
    case '<': mp3.playPrevious(); break;
    case '+': mp3.volUp(); break;
    case '-': mp3.volDown(); break;
    case 'v': mp3.setVol(nval); break;

    case 'c':
      mp3.qPlaying();   // ask current track
      break;
  }
}

void decode_c(){

  if (c=='v' || c=='P' || c=='F' || c=='S'){
    cmd=c;
  }
  else if(c>='0' && c<='9'){
    if(cmd1==' '){
      cmd1 = c;
    }else{
      menu(cmd, ((cmd1-'0')*10)+(c-'0'));
      cmd = ' ';
      cmd1 = ' ';
    }
  }
  else{
    menu(c,0);
  }
}