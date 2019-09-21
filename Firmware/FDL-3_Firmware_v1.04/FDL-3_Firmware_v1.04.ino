/*
 FDL-3 Firmware v1.04 by Project FDL

 Libraries used:
 
 MicroView Arduino Library
 Copyright (C) 2014 GeekAmmo
 
 Encoder Library, for measuring quadrature encoded signals
 http://www.pjrc.com/teensy/td_libs_Encoder.html
 Copyright (c) 2011,2013 PJRC.COM, LLC - Paul Stoffregen <paul@pjrc.com>
*/

#include <MicroView.h>
#include <Encoder.h>
#include <Servo.h>
#include <EEPROM.h>

const float versionNumber = 1.04;
const String splashText = "FDL-3"; //can be two lines split with space

Encoder myEnc(2, 3);
Servo flywheelESC; 

MicroViewWidget *mainGauge;
MicroViewWidget *voltMeter;

#define SAFETY 0
#define ABIDE 1
#define AUTO 2
#define CACHED 3

//PINS
const int pusherBrakePin = 0;
const int pusherEnablePin = 1;
const int escPin = 5;
const int buzzerPin = 6;
const int triggerPin = A0;
const int pusherSwitchPin = A1;
const int lockPin = A2;
const int voltMeterPin = A3;
const int presetBtnPin = A4;

//MENUS
#define batteryCheckLength 6
float batteryCheck[batteryCheckLength];
unsigned long lastBatteryCheck = 0;
unsigned long batteryCheckDelay = 300;
int batteryCheckIndex = 0;
float batteryCheckSum = 0.0;

#define knobMenuLength 15
String knobMenuArray[knobMenuLength] = 
{
  "Speed",
  "ROF",
  "Burst", 
  "MinSpin",
  "MaxSpin", 
  "MinSpd",
  "MaxSpd",
  "BtnMode",
  "FireMode",
  "Spindown",
  "Load",
  "Save",
  "Bright",
  "Sound",
  "Info"  
};
byte knobMenuIndex = 0;

#define burstMenuLength 4
String burstMenuArray[burstMenuLength] = {"1","2","3","F"};
byte burstMenuIndex = 0;

#define soundMenuLength 2
String soundMenuArray[soundMenuLength] = {"ON","OFF"};
byte soundMenuIndex = 0;

#define presetMenuLength 4
String presetMenuArray[presetMenuLength] = {"Back","1","2","3"};
byte presetMenuIndex = 0;

#define btnmodeMenuLength 4
String btnmodeMenuArray[btnmodeMenuLength] = {"PRESET","SPEED ","ROF","BURST"};
byte btnmodeMenuIndex = 3;

#define firemodeMenuLength 4
String firemodeMenuArray[firemodeMenuLength] = {"SAFE","TRIG","AUTO","CACHE"};
byte firemodeMenuIndex = 1;


//variables
int speedValue = 50;
int rofValue = 100;
int brightnessValue = 100;
int minSpeedValue = 0;
int maxSpeedValue = 100;

int minSpinupValue = 300;
int maxSpinupValue = 180;

int dwellTimeValue = 1000;

bool liveKnobScrollMode = false;
bool menuBtnWasDown = false;
bool firstMenuRun = true;
bool speedLocked = false;
bool firstRun = true;
int cacheShots = 0;

int currentSpeed = 1000;
int spindownRate = 12; //units per 10ms
unsigned long lastSpindownCheck = 0;
unsigned long elapsedTime = 0;

unsigned long lastTriggerUp = 0;
unsigned long lastSettingsSave = 0;
unsigned long lastBatAlarm = 0;

long encoderChange = 0;

float vPow = 5.12; //5.27
float r1 = 100000;
float r2 = 10000;

struct BlasterSettings {
  float versionNumber;
  int speedValue;
  int rofValue;
  byte burstCount;
  int minSpinupValue;
  int maxSpinupValue;
  int minSpeed;
  int maxSpeed;
  byte btnMode;
  byte fireMode;
  int brightness;
  bool soundOn;
  int spinDown;
};
BlasterSettings currentSettings = { 1.04, 60, 100, 1, 300, 180, 0, 100, 3, 1, 100, true, 12};
BlasterSettings defaultSettings = { 1.04, 60, 100, 1, 300, 180, 0, 100, 3, 1, 100, true, 12};



void setup() {

  flywheelESC.attach(escPin); 
  flywheelESC.writeMicroseconds(1000);
  
  uView.begin();// start MicroView
  uView.clear(PAGE);// clear page
  uView.display();

  pinMode(voltMeterPin, INPUT);
  pinMode(presetBtnPin, INPUT);
  pinMode(pusherSwitchPin, INPUT);
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(lockPin, INPUT_PULLUP);
  pinMode(pusherEnablePin, OUTPUT);
  pinMode(pusherBrakePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  renderSplash(splashText);

  if(presetButtonDown() == 4){
    
    while(presetButtonDown() == 4){};
    delay(50);
    
    uView.setCursor(0,0);
    uView.print("Click knob");
    uView.setCursor(0,14);
    uView.print("to clear");
    uView.setCursor(0,28);
    uView.print("settings");
    uView.display();

    long currMills = millis();
    while(millis() < currMills + 2000){
      if(presetButtonDown() == 4){
        clearSettings();
        break;
      }
    }
    uView.clear(PAGE);// clear page
    uView.display();
  }

  mainGauge = new MicroViewGauge(16, 30, 0, 100, WIDGETSTYLE0 + WIDGETNOVALUE);
  voltMeter = new MicroViewSlider(56, 14, 109, 126, WIDGETSTYLE2 + WIDGETNOVALUE);

  mainGauge->reDraw();
  voltMeter->reDraw();
  
  //initSettings();
  loadSettings(0);

  //init battery average read values
  for(int x = 0; x < batteryCheckLength; x++){
    batteryCheck[x] = 12.0;
  }
  lastBatteryCheck = millis();
  batteryCheckSum = 12.0 * batteryCheckLength;
  
  renderScreen(true);
  startUpBeeps();


  if(lockOn()){
    delay(500);

    tone(6, 3000, 200);
    delay(300);
    tone(6, 3000, 200);

    speedLocked = true;
  }
}


void loop() {

  //check for trigger
  if(triggerDown() || (cacheShots > 0 && firemodeMenuIndex == CACHED)){
    fireSequence();
  }
  
  spinDown();  
  
  renderScreen(true);

  if(lastSettingsSave != 0 && millis() - lastSettingsSave < 1000){
     //don't save
  }
  else{
     writeSettings(0);
     lastSettingsSave = millis();
  }
  
}

void startUpBeeps(){
  tone(6, 2000, 60);
  delay(110);
  tone(6, 2000, 60);
  delay(110);
  tone(6, 2000, 60);
}















