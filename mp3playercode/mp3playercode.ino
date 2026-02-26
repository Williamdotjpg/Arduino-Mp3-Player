/*
Developers: Will Holmes, Cyle Krohling

resources: https://www.oceanlabz.in/getting-started-with-serial-mp3-player/
https://forum.arduino.cc/t/using-serialmp3player-library-with-yx5300-mp3-player-and-arduino-mega/1194648
http://github.com/salvadorrueda/SerialMP3Player/tree/master




*/



#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mp3(10, 11); // RX, TX

byte queryTrack[] = {0x7E,0xFF,0x06,0x4C,0x00,0x00,0x00,0xFE,0xB0,0xEF};

void setup() {
  lcd.init();
  lcd.backlight();
  mp3.begin(9600);
}

void loop() {
  mp3.write(queryTrack, 10);
  delay(200);

  if (mp3.available() >= 10) {
    byte response[10];
    for (int i = 0; i < 10; i++) {
      response[i] = mp3.read();
    }

    int trackNumber = response[6];  // low byte
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Track: ");
    lcd.print(trackNumber);
  }

  delay(1000);
}
