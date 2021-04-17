
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

#define REST      0

#include <Ultrasonic.h>
#include <LiquidCrystalIO.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
RTC_DS1307 rtc;

//************************************************************************//
//-------------------------------pins-------------------------------------//

Ultrasonic ultrasonic(12, 13); //trigger,echo
#define WRITE_DATA_PIN 11
#define WRITE_LATCH_PIN 10
#define WRITE_CLOCK_PIN 9
const int button = 4;
const int buzzer = 8;
const int chipSelect = 3;

//************************************************************************//


//***********************************************************************//
//-------------------------------variables/consts-------------------------------//
// change this to make the song slower or faster
int tempo = 114;
//distance reading here
int distance;
static int PCOUNTER = 0;//counts posture state in seconds
String POSTURE_STATE="UNDEFINED?";

const int DEBUG_DIST = 21;
const int DEBUG_MODE = 1;//set to 0 when not debugging

const int MIN_RANGE = 0;
const int MID_RANGE= 20;
const int LOW_RANGE= 5;
const int MAX_RANGE= 40;

const int POSTURE_BAD_SEC = 10; //how many samples*seconds to confirm bad posture
//****************************************************************************//
//-------------------------------tone variables-------------------------------//

int melody[] = {

  NOTE_E4,4,NOTE_C5,4,NOTE_B4,4,NOTE_A4,4,REST,4,NOTE_E4,4,NOTE_F4,4,NOTE_G4,4,NOTE_F4,4,NOTE_E4,4,REST,4,REST,4,REST,4,REST,4

};
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;
void AlarmSound(){
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(buzzer);
  }}

//************************************************************************//


//***********************************************************************//
//-----------------------------lcd variables----------------------------//


const int d7 = SHIFT_REGISTER_OUTPUT_CUTOVER + 0;
const int d6 = SHIFT_REGISTER_OUTPUT_CUTOVER + 1;
const int d5 = SHIFT_REGISTER_OUTPUT_CUTOVER + 2;
const int d4 = SHIFT_REGISTER_OUTPUT_CUTOVER + 3;
//const int d6 = SHIFT_REGISTER_OUTPUT_CUTOVER + 4;
//const int d7 = SHIFT_REGISTER_OUTPUT_CUTOVER + 5;
const int en = SHIFT_REGISTER_OUTPUT_CUTOVER + 6;
const int rs = SHIFT_REGISTER_OUTPUT_CUTOVER + 7;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7, outputOnlyFromShiftRegister(WRITE_CLOCK_PIN, WRITE_DATA_PIN, WRITE_LATCH_PIN));

//************************************************************************//
//-----------------------------SD card and RTC setup----------------------------//
//***********************************************************************//
void setupRTC_SD(){
    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    //Serial.flush();
    //abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    //while (1);
  }
  Serial.println("card initialized.");
}

void DataLogging(String &dataString, DateTime &now){
dataString += String(now.timestamp(DateTime::TIMESTAMP_DATE));
    dataString += String(',');
    dataString += String(now.timestamp(DateTime::TIMESTAMP_TIME));
    dataString += String(',');
    dataString += String(POSTURE_STATE);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog7.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");}
}
//***********************************************************************//
//-----------------------------main functions----------------------------//


void setup() {
// set up the LCD's number of columns and rows, must be called.
  Serial.begin(9600);
  lcd.begin(16, 2);
  //setupRTC_SD();
  // Print a message to the LCD.
  pinMode(button, INPUT_PULLUP);
  lcd.print("Tracking Begin");
  
}

void loop() {
  //DateTime now = rtc.now();

  // make a string for assembling the data to log:
  String dataString = "";

  String LAST_POSTURE = POSTURE_STATE;
  if (!DEBUG_MODE)
  distance = ultrasonic.read();
  else distance = DEBUG_DIST;
  if (distance > MID_RANGE){
    //you are standing away

    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Not Sitting");
    Serial.print("Not Sitting");
    POSTURE_STATE = "NOT SEATED";
    Serial.println(POSTURE_STATE);
  }
  else if (distance >=MIN_RANGE && distance <=LOW_RANGE)
  {
    //youre sitting correctly
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Keep going!");
    Serial.print("Keep going!");
    POSTURE_STATE = "GOOD POSTURE";
    Serial.println(POSTURE_STATE);
  }
  if(distance > LOW_RANGE && distance < MID_RANGE){
    PCOUNTER = 1;
    for(int i = 0; i<POSTURE_BAD_SEC;i++){//samples the sensor 1 time per sec to check for bad posture
      delay(1000);
      if (!DEBUG_MODE)
      distance = ultrasonic.read(); //re read
      else distance = DEBUG_DIST;
      
      lcd.setCursor(0,0);
      lcd.clear();
      lcd.print("Alarm in...");

      if (distance >= LOW_RANGE && distance<MID_RANGE){
          PCOUNTER++;
          lcd.setCursor(0,1);
          lcd.print(PCOUNTER);
      }
      else break;
    } 
    if (PCOUNTER > POSTURE_BAD_SEC){
        
        lcd.setCursor(0,0);
        lcd.clear();
        lcd.print("Please fix your");
        lcd.setCursor(0,1);
        lcd.print("posture!");
        int duration = millis();
        AlarmSound();// sounds the alarm
        duration = millis() - duration; //calculating alarm duration in seconds
        PCOUNTER =+ duration/1000;
        POSTURE_STATE = "BAD POSTURE";
        Serial.println(POSTURE_STATE);

    }
  }
  if (LAST_POSTURE == POSTURE_STATE){
    lcd.setCursor(0,1);
    PCOUNTER++;
    lcd.print(PCOUNTER);
    delay(1000);
    //DataLogging(dataString,now);
  }
  else
  {
    PCOUNTER = 0;
  }
  
  
  delay(1000);
}