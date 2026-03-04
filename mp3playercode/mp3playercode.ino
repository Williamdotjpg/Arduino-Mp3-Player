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

SerialMP3Player mp3(RX, TX);
LiquidCrystal_I2C lcd(0x27, 16, 2);

char c;
char cmd=' ';
char cmd1=' ';
int lastVolume = -1;

void menu(char op, int nval);
void decode_c();
void checkButtons();

void setup() {
  Serial.begin(9600);
  mp3.begin(9600);
  delay(500);

  mp3.sendCommand(CMD_SEL_DEV, 0, 2);
  delay(500);

  pinMode(BTN_NEXT,      INPUT_PULLUP);
  pinMode(BTN_PREV,      INPUT_PULLUP);
  pinMode(BTN_PLAYPAUSE, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("MP3 Ready");
}

void loop() {
  static unsigned long lastQuery = 0;

  // Potentiometer volume
  int potValue = analogRead(POT);
  int volume = map(potValue, 0, 1023, 0, 30);
  if (volume != lastVolume) {
    mp3.setVol(volume);
    lastVolume = volume;
    lcd.setCursor(0,1);
    lcd.print("Vol: ");
    lcd.print(volume);
    lcd.print("   ");
  }

  if (millis() - lastQuery > 2000) {
    mp3.qPlaying();
    lastQuery = millis();
  }

  checkButtons();

  if (Serial.available()) {
    c = Serial.read();
    decode_c();
  }

  if (mp3.available()) {
    String answer = mp3.decodeMP3Answer();
    Serial.println(answer);
    if (answer.indexOf("Playing:") >= 0) {
      int track = answer.substring(answer.lastIndexOf(" ") + 1).toInt();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Track: ");
      lcd.print(track);
      lcd.setCursor(0,1);
      lcd.print("Vol: ");
      lcd.print(lastVolume);
    }
  }
}

void checkButtons() {
  const unsigned long DOUBLE_CLICK_WINDOW = 400;
  const unsigned long DEBOUNCE = 50;

  static unsigned long lastNextBounce  = 0;
  static unsigned long lastPrevBounce  = 0;
  static unsigned long lastPlayBounce  = 0;
  static unsigned long lastNextPress   = 0;
  static unsigned long lastPrevPress   = 0;
  static bool nextPending = false;
  static bool prevPending = false;
  static bool isPlaying   = true;

  unsigned long now = millis();

  // --- NEXT button ---
  if (digitalRead(BTN_NEXT) == LOW && now - lastNextBounce > DEBOUNCE) {
    lastNextBounce = now;

    if (nextPending && (now - lastNextPress < DOUBLE_CLICK_WINDOW)) {
      // Double click → skip to next track
      mp3.playNext();
      nextPending = false;
      isPlaying = true;
      lcd.setCursor(0,0);
      lcd.print(">> Next Track   ");
    } else {
      // First click — wait to see if double click follows
      nextPending = true;
      lastNextPress = now;
    }

    while (digitalRead(BTN_NEXT) == LOW);
  }

  // Single click timeout → forward ~10 seconds
  if (nextPending && (now - lastNextPress > DOUBLE_CLICK_WINDOW)) {
    mp3.sendCommand(0x1C, 0, 0);
    nextPending = false;
    lcd.setCursor(0,0);
    lcd.print("+10s            ");
  }

  // --- PREV button ---
  if (digitalRead(BTN_PREV) == LOW && now - lastPrevBounce > DEBOUNCE) {
    lastPrevBounce = now;

    if (prevPending && (now - lastPrevPress < DOUBLE_CLICK_WINDOW)) {
      // Double click → go to previous track
      mp3.playPrevious();
      prevPending = false;
      isPlaying = true;
      lcd.setCursor(0,0);
      lcd.print("<< Prev Track   ");
    } else {
      prevPending = true;
      lastPrevPress = now;
    }

    while (digitalRead(BTN_PREV) == LOW);
  }

  // Single click timeout → rewind ~10 seconds
  if (prevPending && (now - lastPrevPress > DOUBLE_CLICK_WINDOW)) {
    mp3.sendCommand(0x1B, 0, 0);
    prevPending = false;
    lcd.setCursor(0,0);
    lcd.print("-10s            ");
  }

  // --- PLAY/PAUSE button ---
  if (digitalRead(BTN_PLAYPAUSE) == LOW && now - lastPlayBounce > DEBOUNCE) {
    lastPlayBounce = now;

    if (isPlaying) {
      mp3.pause();
      isPlaying = false;
      lcd.setCursor(0,0);
      lcd.print("Paused          ");
    } else {
      mp3.play();
      isPlaying = true;
      lcd.setCursor(0,0);
      lcd.print("Playing         ");
    }

    while (digitalRead(BTN_PLAYPAUSE) == LOW);
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
    case 'c': mp3.qPlaying(); break;
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
