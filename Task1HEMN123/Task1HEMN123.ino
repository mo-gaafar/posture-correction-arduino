
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


//************************************************************************//
//-------------------------------pins-------------------------------------//

Ultrasonic ultrasonic(12, 13); //trigger,echo
#define WRITE_DATA_PIN 11
#define WRITE_LATCH_PIN 10
#define WRITE_CLOCK_PIN 9
const int button = 4;
const int buzzer = 8;

//************************************************************************//


//***********************************************************************//
//-------------------------------variables-------------------------------//
// change this to make the song slower or faster
int tempo = 114;
//distance reading here
int distance;
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
void setup() {
// set up the LCD's number of columns and rows, must be called.
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Posture Tracking");
  Serial.begin(9600);
}

void loop() {
  distance = ultrasonic.read();
  if (distance > 20){
    //you are standing away
    pinMode(button, INPUT_PULLUP);
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Not Sitting");

    //sd card print not sitting + timestamp
  }
  else if (distance >0 && distance <=5)
  {
    //youre sitting correctly
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Keep going!");
    //sd card print good posture + timestamp
  }
  if(distance > 5 && distance < 20){
    int badposturecount = 0;
    for(int i = 0; i<10;i++){
      delay(1000);
      distance = ultrasonic.read(); //re read
      if (distance >= 5 && distance<20){
          badposturecount++;
      }
      else break;
    } 
    if (badposturecount == 10){
        
        lcd.setCursor(0,0);
        lcd.clear();
        lcd.print("Please fix your");
        lcd.setCursor(0,1);
        lcd.print("posture!");
        AlarmSound();// sounds the alarm
        
        //sd card print bad posture + timestamp

    }
  }
  delay(100);
}