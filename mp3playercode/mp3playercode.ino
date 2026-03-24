/*
Developers: Will Holmes, Cyle Krohling, and Kiera McKimmy

resources: https://www.oceanlabz.in/getting-started-with-serial-mp3-player/
https://forum.arduino.cc/t/using-serialmp3player-library-with-yx5300-mp3-player-and-arduino-mega/1194648
http://github.com/salvadorrueda/SerialMP3Player/tree/master

LCD Troubleshooting Code:https://chatgpt.com/share/69a5dca7-a304-8002-a3b0-b455103b4850
B10K: https://chatgpt.com/share/69a5e2ae-5de0-8002-92cb-9df7dc2da5c6
Buttons: https://claude.ai/share/514c4088-8de8-4913-9fd6-031d9d5ecccd
https://docs.arduino.cc/built-in-examples/digital/Button/ 

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
#include "SerialMP3Player.h"  // Library for controlling the MP3 module
#include <Wire.h>             // I2C communication library (used by LCD)
#include <LiquidCrystal_I2C.h> // Library for the I2C LCD display

// --- Pin definitions ---
#define TX 3       // MP3 module TX pin
#define RX 2       // MP3 module RX pin
#define POT A0     // Potentiometer for volume control

// --- Button pins ---
#define BTN_NEXT       A1  // Skip to next track
#define BTN_PLAYPAUSE  A2  // Play or pause current track
#define BTN_PREV       A3  // Go to previous track

// Create MP3 player object using defined RX/TX pins
SerialMP3Player mp3(RX, TX);

// Create LCD object — 0x27 is the I2C address, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables used for reading serial commands
char c;       // Current character read from serial
char cmd=' '; // Stores command type (v, P, F, S)
char cmd1=' '; // Stores first digit of a two-digit number

int lastVolume = -1; // Tracks last volume so we only update when it changes

// Forward declarations so functions can be called before they're defined
void menu(char op, int nval);
void decode_c();
void checkButtons();

void setup() {
  Serial.begin(9600);   // Start serial communication for debugging
  mp3.begin(9600);      // Start MP3 module at 9600 baud
  delay(500);           // Wait for MP3 module to initialize

  mp3.sendCommand(CMD_SEL_DEV, 0, 2); // Tell MP3 module to use SD card
  delay(500);           // Wait for SD card to mount

  // Set button pins as inputs with internal pull-up resistors
  // Pins read HIGH by default, LOW when button is pressed
  pinMode(BTN_NEXT,      INPUT_PULLUP);
  pinMode(BTN_PREV,      INPUT_PULLUP);
  pinMode(BTN_PLAYPAUSE, INPUT_PULLUP);

  // Initialize LCD and show startup message
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);   // Move cursor to top left
  lcd.print("MP3 Ready");
}

void loop() {

  // --- Potentiometer Volume Control ---
  int potValue = analogRead(POT);                    // Read raw value (0-1023)
  int volume = map(potValue, 0, 1023, 0, 30);        // Scale to MP3 volume range (0-30)

  if (volume != lastVolume) {                        // Only update if volume changed
    mp3.setVol(volume);                              // Send new volume to MP3 module
    lastVolume = volume;                             // Save new volume
    lcd.setCursor(0,1);                              // Move to second row
    lcd.print("Vol: ");
    lcd.print(volume);
    lcd.print("   ");                                // Blank out leftover characters
  }

  checkButtons(); // Check if any buttons are being pressed

  // --- Serial Input ---
  if (Serial.available()) {    // If a character was sent via Serial Monitor
    c = Serial.read();         // Read the character
    decode_c();                // Process it as a command
  }

  // --- MP3 Module Response ---
  if (mp3.available()) {                            // If the MP3 module sent a response
    String answer = mp3.decodeMP3Answer();          // Decode the response
    Serial.println(answer);                         // Print it to Serial Monitor

    if (answer.indexOf("Playing:") >= 0) {          // If response contains track info
      // Extract track number from the end of the response string
      int track = answer.substring(answer.lastIndexOf(" ") + 1).toInt();

      // Update top row with track number
      lcd.setCursor(0,0);
      lcd.print("Track: ");
      lcd.print(track);
      lcd.print("   ");       // Blank out leftover characters

      // Update bottom row with current volume
      lcd.setCursor(0,1);
      lcd.print("Vol: ");
      lcd.print(lastVolume);
      lcd.print("   ");
    }
  }
}

void checkButtons() {
  const unsigned long COOLDOWN = 100;      // Minimum ms between button triggers
  const unsigned long MSG_DURATION = 1000; // How long action message stays on LCD (ms)

  static unsigned long lastNextPress  = 0; // Timestamp of last next button press
  static unsigned long lastPrevPress  = 0; // Timestamp of last prev button press
  static unsigned long lastPlayPress  = 0; // Timestamp of last play/pause button press
  static unsigned long msgShownAt     = 0; // Timestamp of when action message appeared
  static bool msgActive = false;           // Whether an action message is currently showing
  static bool isPlaying = true;            // Tracks whether music is playing or paused

  unsigned long now = millis(); // Get current time in milliseconds

  // --- Action Message Timeout ---
  // After MSG_DURATION ms, query the current track to refresh the LCD
  if (msgActive && now - msgShownAt > MSG_DURATION) {
    msgActive = false;   // Mark message as done
    mp3.qPlaying();      // Ask MP3 module for current track info
  }

  // --- NEXT button ---
  // Fires if button is pressed and enough time has passed since last press
  if (digitalRead(BTN_NEXT) == LOW && now - lastNextPress > COOLDOWN) {
    lastNextPress = now;             // Record press time
    mp3.playNext();                  // Skip to next track
    isPlaying = true;                // Mark as playing
    lcd.setCursor(0,0);
    lcd.print(">> Next Track   ");  // Show action message on LCD
    msgActive = true;                // Start message timer
    msgShownAt = now;
  }

  // --- PREV button ---
  if (digitalRead(BTN_PREV) == LOW && now - lastPrevPress > COOLDOWN) {
    lastPrevPress = now;             // Record press time
    mp3.playPrevious();              // Go to previous track
    isPlaying = true;                // Mark as playing
    lcd.setCursor(0,0);
    lcd.print("<< Prev Track   ");  // Show action message on LCD
    msgActive = true;                // Start message timer
    msgShownAt = now;
  }

  // --- PLAY/PAUSE button ---
  if (digitalRead(BTN_PLAYPAUSE) == LOW && now - lastPlayPress > COOLDOWN) {
    lastPlayPress = now;             // Record press time

    if (isPlaying) {
      mp3.pause();                   // Pause playback
      isPlaying = false;             // Update state
      lcd.setCursor(0,0);
      lcd.print("Paused          "); // Show paused message (no timer needed, stays until next action)
    } else {
      mp3.play();                    // Resume playback
      isPlaying = true;              // Update state
      lcd.setCursor(0,0);
      lcd.print("Playing         "); // Show playing message
      msgActive = true;              // Start message timer to return to track info
      msgShownAt = now;
    }
  }
}

// --- Menu function ---
// Executes MP3 commands based on a character code and optional numeric value
void menu(char op, int nval){
  switch (op){
    case 'P': mp3.play(nval); break;       // Play specific track number
    case 'F': mp3.playF(nval); break;      // Play specific file number
    case 'S': mp3.playSL(nval); break;     // Play track in folder
    case 'p': mp3.play(); break;           // Play current track
    case 'a': mp3.pause(); break;          // Pause
    case 's': mp3.stop(); break;           // Stop
    case '>': mp3.playNext(); break;       // Next track
    case '<': mp3.playPrevious(); break;   // Previous track
    case '+': mp3.volUp(); break;          // Volume up
    case '-': mp3.volDown(); break;        // Volume down
    case 'v': mp3.setVol(nval); break;     // Set specific volume
    case 'c': mp3.qPlaying(); break;       // Query current track
  }
}

// --- decode_c function ---
// Parses characters from Serial Monitor into commands for the menu function
void decode_c(){
  if (c=='v' || c=='P' || c=='F' || c=='S'){
    cmd=c;             // Store command type, wait for number input
  }
  else if(c>='0' && c<='9'){
    if(cmd1==' '){
      cmd1 = c;        // Store first digit
    }else{
      // Both digits received — combine into a number and execute command
      menu(cmd, ((cmd1-'0')*10)+(c-'0'));
      cmd = ' ';       // Reset command
      cmd1 = ' ';      // Reset first digit
    }
  }
  else{
    menu(c,0);         // Single character command with no number (e.g. play, pause, stop)
  }
}