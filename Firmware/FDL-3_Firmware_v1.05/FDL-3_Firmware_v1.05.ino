/*
 FDL-3 Firmware v1.05 by Project FDL
                v1.05.1 by Mathew3000
 Libraries used:
 
 Adafruit SSD1306 Library
 https://github.com/adafruit/Adafruit_SSD1306
 Copyright (c) 2012, Adafruit Industries. All rights reserved.
  
 Adafruit GFX Library
 https://github.com/adafruit/Adafruit-GFX-Library
 Copyright (c) 2012, Adafruit Industries. All rights reserved.

 
 Encoder Library, for measuring quadrature encoded signals
 http://www.pjrc.com/teensy/td_libs_Encoder.html
 Copyright (c) 2011,2013 PJRC.COM, LLC - Paul Stoffregen <paul@pjrc.com>
*/

#include <SPI.h>
#include <Wire.h>
#include <Encoder.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include <Servo.h>

const byte versionNumber = 105;
const String splashText = "FDL-3 Arduino"; //can be two lines split with space

Servo flywheelESC; 
Encoder myEnc(2, 3); 
//MicroViewWidget *mainGauge;
//MicroViewWidget *voltMeter;
//PARTY

// OLED DEF
#define OLED_DC     8
#define OLED_CS     10
#define OLED_RESET  7
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_HEADER 16
#define SMALL_T     1
#define LARGE_T     2
#define CH_SH       7
#define CH_LH       14
#define CH_SW       5
#define CH_LW       10
Adafruit_SSD1306 oled(OLED_WIDTH,OLED_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS, 4000000UL);

#define NOTHROTTLE 1000
#define MINTHROTTLE 1285
#define MAXTHROTTLE 2000

//PINS
#define pusherBrakePin 0
#define pusherEnablePin 1
#define escPin 5
#define buzzerPin 6
#define triggerPin A0
#define pusherSwitchPin A1
#define lockPin A2
#define voltMeterPin A3
#define presetBtnPin A4

//MENUS
const char *knobMenu[] = 
{
  "Speed",
  "ROF",
  "Burst", 
  "MinSpin",
  "MaxSpin",   
  "FireMode",
  "Spindown",
  "Idle",
  "Load",
  "Save",
  "MinSpd",
  "MaxSpd",
  "BtnMode",
  "BrkAg",
  "ULock",
  "Bright",
  "Sound",
  "BatOff",
  "Info"    
};
byte knobMenuIndex = 0;

const char *burstMenu[] = {"1","2","3","F"};
const char *soundMenu[] = {"OFF","ON"};
const char *presetMenu[] = {"Back","1","2","3"};
const char *btnmodeMenu[] = {"PRESET","SPEED","ROF","BURST"};
const char *firemodeMenu[] = {"SAFE","TRIG","AUTO","CACHE"};
byte presetMenuIndex = 0;

//variables
bool liveKnobScrollMode = false;
bool menuBtnWasDown = false;
bool firstMenuRun = true;
bool speedLocked = false;
bool firstRun = true;
unsigned long lastSpinCheck = 0;
unsigned long lastSettingsSave = 0;
unsigned long lastBatAlarm = 0;
unsigned long brakeRelease = 0;
unsigned long idleRelease = 0;
long encoderChange = 0;
int currentSpeed = NOTHROTTLE;
int spindownTarget = NOTHROTTLE;

#define batteryCheckLength 6
float batteryCheck[batteryCheckLength];
unsigned long lastBatteryCheck = 0;
int batteryCheckIndex = 0;
float batteryCheckSum = 0.0;

struct StaticSettings {
  byte btnMode;
  int brightness;
  byte soundOn;
  int batOffset;
  int brkAgr;
  int usrLock;
  int minSpeed;
  int maxSpeed;
};
StaticSettings currStSettings = { 3, 100, true, 0, 16, 0, 0, 100 };
StaticSettings lastStSettings = { 3, 100, true, 0, 16, 0, 0, 100 };
StaticSettings defStSettings = { 3, 100, true, 0, 16, 0, 0, 100 };

struct BlastSettings {
  int speedValue;
  int rofValue;
  byte burstCount;
  int minSpinup;
  int maxSpinup;
  int spinDown; //units per ms for single value drop (higher = slower)
  int idleTime;
  byte fireMode;  
};
BlastSettings currBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };
BlastSettings lastBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };
BlastSettings readBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };
BlastSettings defBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };

void setup() {
  oled.begin();
  oled.clearDisplay();
  oled.display();
  oled.setTextSize(LARGE_T);   
  oled.setTextColor(SSD1306_WHITE); // Draw white text
  oled.setCursor(0, 0);     // Start at top-left corner
  oled.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Setup ESC
  flywheelESC.attach(escPin); 
  flywheelESC.writeMicroseconds(0);  

  // Setup Pin Modes
  pinMode(voltMeterPin, INPUT);
  pinMode(presetBtnPin, INPUT);
  pinMode(pusherSwitchPin, INPUT);
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(lockPin, INPUT_PULLUP);
  pinMode(pusherEnablePin, OUTPUT);
  pinMode(pusherBrakePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  oled.clearDisplay();
  renderSplash(splashText);  
  loadSettings();
  
  if(presetButtonDown() == 4){
    int tmpLocVal = currStSettings.usrLock;
    clearSetRoutine();
    currStSettings.usrLock = tmpLocVal;
    writeStaticSettings();
  }
  
  if(lockOn() && presetButtonDown() == 3){
    clearLockRoutine();
  }

  //PARTY
  //mainGauge = new MicroViewGauge(16, 30, 0, 100, WIDGETSTYLE0 + WIDGETNOVALUE);
  //voltMeter = new MicroViewSlider(56, 14, 106, 126, WIDGETSTYLE2 + WIDGETNOVALUE);
  //mainGauge->reDraw();
  //voltMeter->reDraw();

  initBatteryCheck(); 

  if(currStSettings.usrLock != 0){
    renderUserLock();
  }
  else{
    flywheelESC.writeMicroseconds(NOTHROTTLE);
    
    renderScreen();
    startUpBeeps();
    
    if(lockOn()){
      delay(500);
      toneAlt(3000, 150);
      delay(150);
      toneAlt(3000, 150);
      speedLocked = true;
    }
  }
}

void loop() {

  if(currStSettings.usrLock != 0){
    renderUserLock();
    return;
  }

  //cap speed at max
  if(currBlSettings.speedValue > currStSettings.maxSpeed){
    currBlSettings.speedValue = currStSettings.maxSpeed;
  }
  
  //check for trigger
  if(triggerDown() && millis() > 2800){
    fireSequence();
  }
  else{
    if(millis() > brakeRelease){
      digitalWrite(pusherBrakePin, LOW); 
      delayMicroseconds(200);
    }
    if(millis() > idleRelease){
      spindownTarget = NOTHROTTLE; 
    }
  }
   
  spinDownFW(false);  
  renderScreen();

  if(lastSettingsSave != 0 && millis() - lastSettingsSave < 2000){
     //don't save
  }
  else{
    if(currentSettingsChanged() || staticSettingsChanged()){
      // PARTY
      writeCurrentSettings();
      writeStaticSettings();
      lastSettingsSave = millis();      
    }
  }
}
