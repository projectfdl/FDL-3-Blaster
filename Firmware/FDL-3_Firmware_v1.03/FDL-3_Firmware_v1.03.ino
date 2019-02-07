#include <MicroView.h>
#include <Encoder.h>
#include <Servo.h>
#include <EEPROM.h>

float versionNumber = 1.03;

Encoder myEnc(2, 3);
Servo flywheelESC; 

//PINS
int pusherBrakePin = 0;
int pusherEnablePin = 1;
int escPin = 5;
int buzzerPin = 6;
int triggerPin = A0;
int pusherSwitchPin = A1;
int lockPin = A2;
int voltMeterPin = A3;
int presetBtnPin = A4;

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



MicroViewWidget *mainGauge;
MicroViewWidget *voltMeter;



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
BlasterSettings currentSettings = { 1.02, 60, 100, 1, 300, 180, 0, 100, 3, 1, 100, true, 8};
BlasterSettings defaultSettings = { 1.02, 60, 100, 1, 300, 180, 0, 100, 3, 1, 100, true, 8};

bool trgReleased = false;
int cacheShots = 0;

int currentSpeed = 1000;
int spindownRate = 12; //units per 10ms
unsigned long lastSpindownCheck = 0;
unsigned long elapsedTime = 0;

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

  renderSplash();

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

  mainGauge = new MicroViewGauge(16,30,0,100,WIDGETSTYLE0 + WIDGETNOVALUE);
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
  //delay(50);
  startUpBeeps();


  if(lockOn()){
    delay(500);

    tone(6, 3000, 200);
    delay(300);
    tone(6, 3000, 200);

    speedLocked = true;
  }
}

void startUpBeeps(){
  tone(6, 2000, 60);
  delay(110);
  tone(6, 2000, 60);
  delay(110);
  tone(6, 2000, 60);
}


void loop() {

  //check for trigger
  if(triggerDown() || (cacheShots > 0 && firemodeMenuIndex == 3)){
    fireSequence();
  }
  else{
    spinDown();
  }
  
  renderScreen(true);

  if(lastSettingsSave != 0 && millis() - lastSettingsSave < 1000){
     //don't save
  }
  else{
     writeSettings(0);
     lastSettingsSave = millis();
  }
  
}

void writeSettings(int presetNumber){

//struct BlasterSettings {
//  float versionNumber;
//  byte speedValue;
//  byte rofValue;
//  byte burstCount;
//  int minSpinupValue;
//  int maxSpinupValue;
//  byte minSpeed;
//  byte maxSpeed;
//  byte btnMode;
//  byte fireMode;
//  int dwellTime;
//  byte brightness;
//  bool soundOn;
//};

  if((currentSettings.versionNumber != versionNumber||
  currentSettings.speedValue != speedValue ||
  currentSettings.rofValue != rofValue ||
  currentSettings.burstCount != getBurstCount() ||
  currentSettings.minSpinupValue != minSpinupValue ||
  currentSettings.maxSpinupValue != maxSpinupValue ||
  currentSettings.minSpeed != minSpeedValue ||
  currentSettings.maxSpeed != maxSpeedValue ||
  currentSettings.btnMode != btnmodeMenuIndex ||
  currentSettings.fireMode != firemodeMenuIndex ||
  currentSettings.brightness != brightnessValue ||
  currentSettings.soundOn != (soundMenuIndex == 0)) ||
  presetNumber > 0 ||
  currentSettings.spinDown != spindownRate){
    currentSettings = { 
      versionNumber, 
      speedValue,
      rofValue,
      getBurstCount(),
      minSpinupValue,
      maxSpinupValue,
      minSpeedValue,
      maxSpeedValue,
      btnmodeMenuIndex,
      firemodeMenuIndex,
      brightnessValue,
      soundMenuIndex == 0,
      spindownRate};

    int memLoc = presetNumber * 100;
      
    EEPROM.put(memLoc, currentSettings);
    
    
    if(presetNumber == 0){
      //tone(6,3000,50);
    }
  }
}

void loadSettings(int presetNumber){

  int memLoc = presetNumber * 100;
  EEPROM.get(memLoc, currentSettings);
    
  //if EEPROM empty all values will come back -1
  if(currentSettings.versionNumber == -1 || currentSettings.versionNumber != versionNumber){
    currentSettings = defaultSettings;

    tone(6, 1000, 50);
    delay(100);
    tone(6, 1000, 50);
  }
  
  speedValue = currentSettings.speedValue;
  rofValue = currentSettings.rofValue;
  
  int readBurst = constrain(currentSettings.burstCount, 1, 4);
  burstMenuIndex = readBurst - 1;
  
  minSpinupValue = currentSettings.minSpinupValue;
  maxSpinupValue = currentSettings.maxSpinupValue;
  minSpeedValue = currentSettings.minSpeed;
  maxSpeedValue = currentSettings.maxSpeed;
  brightnessValue = currentSettings.brightness;
  
  spindownRate = currentSettings.spinDown;
  
  firemodeMenuIndex = currentSettings.fireMode;

  if(presetNumber == 0){
    soundMenuIndex = currentSettings.soundOn ? 0 : 1;
    btnmodeMenuIndex = currentSettings.btnMode;
  }
  
}

void savePreset(int presetIndex){

  writeSettings(presetIndex);
  
  tone(6, 2000, 50);
  delay(100);
  tone(6, 2000, 100);
  
}

void loadPreset(int presetIndex){

  loadSettings(presetIndex);
  
  tone(6, 4000, 50);
  delay(100);
  tone(6, 4000, 100);
  
}

void clearSettings(){

  uView.clear(PAGE);// clear page
  uView.setCursor(0,0);
  uView.print("Clearing");
  uView.setCursor(0,14);
  uView.print("settings");
  uView.display();
  
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }

  tone(6, 3000, 50);
  delay(100);
  tone(6, 2500, 100);
  delay(100);
  tone(6, 2000, 200);
  delay(200);
  
}



void triggerLogic(){
  
  if(firemodeMenuIndex == 3){ // in cache mode
    if(!trgReleased && !triggerDown()){
      trgReleased = true;
    }
    if(trgReleased && triggerDown()){
      //tone(6, 2000, 50);
      cacheShots++;
      trgReleased = false;
    }
  }
  
}

bool enterAutoLoop(){

  if(cacheShots > 1){
    if(inFullAuto()){//full auto always abide by trigger
      return triggerDown();
    }
    if(firemodeMenuIndex == 1){//trigger abide mode
      return triggerDown();
    }
    if(firemodeMenuIndex == 2){//auto mode
      return true;
    }
    if(firemodeMenuIndex == 3){//cached mode
      return true;
    }
  }
  
  return false;
}

bool inFullAuto(){
  return getBurstCount() == 100;
}


bool fireSequence(){

    if(firemodeMenuIndex == 0){
      tone(6, 3000, 50);
      delay(100);
      tone(6, 2000, 100);
      return true;
    }

    if(firemodeMenuIndex != 3 || inFullAuto()){
      cacheShots = 0;
    }
    
    long currMills = millis();
    //cacheShots = 0;
    
    int kickonSpeed = readESCPower(); 
    currentSpeed = kickonSpeed;
    flywheelESC.writeMicroseconds(kickonSpeed);

    int spinupDelay = getSpinup();
    cacheShots += getBurstCount();

    trgReleased = false;
    
    for(int i = 0; i < spinupDelay; i++){
      triggerLogic();
      delay(1);
    }

    long counter = 0;

    if(!triggerDown() && firemodeMenuIndex == 1){
      digitalWrite(pusherEnablePin, LOW);
      renderScreen(false);
      lastTriggerUp = millis();

      //delay(100);
      //flywheelESC.writeMicroseconds(kickonSpeed);

      elapsedTime = 0;
      lastSpindownCheck = millis();
      spinDown();
      
      return true;
    }

    //more than one shot cached
    while(enterAutoLoop()){
      
        counter = 50;
        //while front not down, run
        while(pusherSwitchDown() != 1 && counter > 0){
            digitalWrite(pusherEnablePin, HIGH);
            delay(6);
            counter--;
            triggerLogic();
        }
        
        counter = 50;
        //while front down, run
        while(pusherSwitchDown() == 1 && counter > 0){
            digitalWrite(pusherEnablePin, HIGH);
            delay(6);
            counter--;
            triggerLogic();
        }
        
        int burstBreakDelay = map(rofValue, 0, 100, 100, 0);
        
        cacheShots--;
        
        if(burstBreakDelay > 5){
            digitalWrite(pusherEnablePin, LOW);
            delayMicroseconds(200);
            digitalWrite(pusherBrakePin, HIGH);
            delay(burstBreakDelay);
            digitalWrite(pusherBrakePin, LOW);
            delayMicroseconds(200);
        }
        
        renderScreen(false);
    }

    counter = 50;
    //while front not down, run
    while(pusherSwitchDown() != 1 && counter > 0){
        digitalWrite(pusherEnablePin, HIGH);
        delay(6);
        counter--;
        triggerLogic();
    }
    
    counter = 50;
    //while front down, run
    while(pusherSwitchDown() == 1 && counter > 0){
        digitalWrite(pusherEnablePin, HIGH);
        delay(6);
        counter--;
        triggerLogic();
    }

    cacheShots--;

    //if(cacheShots == 0 || inFullAuto() || firemodeMenuIndex != 3){

      cacheShots = 0;
      
      digitalWrite(pusherEnablePin, LOW);
      
      delayMicroseconds(500);
      
      digitalWrite(pusherBrakePin, HIGH);
       for(int i = 0; i < 30; i++){
          triggerLogic();
          delay(1);
       }
      digitalWrite(pusherBrakePin, LOW);
      
      delayMicroseconds(500);
  
      digitalWrite(pusherEnablePin, HIGH);
      delayMicroseconds(16000);
      digitalWrite(pusherEnablePin, LOW);
      
      counter = 1000;
      //while rear not down, coast till hit
      while(pusherSwitchDown() != 2 && counter > 0){
          delayMicroseconds(200);
          counter--;
      }
      
      digitalWrite(pusherEnablePin, LOW);
      //flywheelESC.writeMicroseconds(1000);
      
  
      delayMicroseconds(200);
      
      digitalWrite(pusherBrakePin, HIGH);
      for(int i = 0; i < 30; i++){
        triggerLogic();
        delay(1);
      }
      digitalWrite(pusherBrakePin, LOW);
      
      delayMicroseconds(200);

      lastTriggerUp = millis();
    //}

    cacheShots == 0;

    renderScreen(false);

    elapsedTime = 0;
    lastSpindownCheck = millis();
    
    while(triggerDown() && cacheShots == 0){
      spinDown();  
    };
    
    //lastTriggerUp = millis();
    
    //noTone(6);

    
    
    return true;
}

void spinDown(){

    int flipSpindown = (16 - spindownRate) * 2;

  
    if(currentSpeed > 1000){
      if(spindownRate == 0){
        currentSpeed = 1000;
      }
      else{
        elapsedTime = lastSpindownCheck == 0 ? 0 : elapsedTime + millis() - lastSpindownCheck;
        lastSpindownCheck = millis();
        int spindown = elapsedTime / 10 * flipSpindown;
        elapsedTime %= 10;
        
        currentSpeed -= spindown;
      }
    }
    
    flywheelESC.writeMicroseconds(currentSpeed);
}

boolean triggerDown(){
  return digitalRead(triggerPin) == LOW;
}

boolean lockOn(){
  return digitalRead(lockPin) == LOW;
}

int presetButtonDown(){
  int readVal = analogRead(presetBtnPin);

  if(readVal < 400){
    return 0;
  }
  if(readVal < 700){
    return 4;
  }
  if(readVal < 800){
    return 3;
  }
  if(readVal < 900){
    return 2;
  }
  return 1;
}

int pusherSwitchDown(){
  int readVal = analogRead(pusherSwitchPin);

  if(readVal < 400){
    return 0;
  }
  if(readVal < 900){
    return 1;
  }
  return 2;
}

int readESCPower(){
  return map(speedValue, 0, 100, 1285, 2000);
}

int getSpinup(){

  double minSpin = 0;
  double spinup = map(speedValue, 0, 100, minSpinupValue, maxSpinupValue);
  
  double sinceLastTrigUp = min(millis() - lastTriggerUp, dwellTimeValue);
  double dwellFactor = sinceLastTrigUp / dwellTimeValue;

  double calcSpin = (spinup - minSpin) * dwellFactor + minSpin;

//  uView.rectFill(20, 12, 40, 24, BLACK, NORM);
//  uView.setCursor(20,12);
//  uView.print(calcSpin,0);

  return calcSpin;
}

int getBurstCount(){
  if(burstMenuIndex >= burstMenuLength - 1){
    return 100;
  }
  else{
    return burstMenuIndex + 1;
  }
}

int getROF(){
  return rofValue;
}



/////////////
///RENDER
/////////////
void renderScreen(bool useAlarm){
  
  int contrastValue = map(brightnessValue, 0, 100, 0, 255);
  uView.contrast(contrastValue);

  if(speedLocked){
    renderLockIndicator();
  }
  
  renderVoltMeter(useAlarm);
    
  if(liveKnobScrollMode){
    renderKnobScrollMenu();
  }
  else{
    switch(knobMenuIndex){
      case 0:
        renderGauge(speedValue, "Speed", 0, 100, minSpeedValue, maxSpeedValue, 1);
        break;
      case 1:
        renderGauge(rofValue, "ROF", 0, 100, 0, 100, 1);
        break;
      case 2:
        renderMenu(burstMenuIndex, "Burst", burstMenuArray, burstMenuLength);
        break;
      case 3:
        if(!speedLocked){
          renderGauge(minSpinupValue, "Spn Min", 150, 500, 180, 500, 1); //250, 500, 250, 500);
        }
        break;
      case 4:
        if(!speedLocked){
          renderGauge(maxSpinupValue, "Spn Max", 150, 500, 180, 500, 1);
        }
        break;
      case 5:
        if(!speedLocked){
          renderGauge(minSpeedValue, "Spd Min", 0, 100, 0, maxSpeedValue, 1);
        }
        break;
      case 6:
        if(!speedLocked){
          renderGauge(maxSpeedValue, "Spd Max", 0, 100, minSpeedValue, 100, 1);
        }
        break;
      case 7:
        renderMenu(btnmodeMenuIndex, "Btn Mode", btnmodeMenuArray, btnmodeMenuLength);
        break;
      case 8:
        renderMenu(firemodeMenuIndex, "FireMode", firemodeMenuArray, firemodeMenuLength);
        break;
      case 9:
        renderGauge(spindownRate, "Spindown ", 0, 15, 0, 15, 8);
        break;
      case 10:
        if(!speedLocked){
          renderMenu(presetMenuIndex, "Load", presetMenuArray, presetMenuLength);
        }
        break;
      case 11:
        if(!speedLocked){
          renderMenu(presetMenuIndex, "Save", presetMenuArray, presetMenuLength);
        }
        break;
      case 12:
        renderGauge(brightnessValue, "Bright ", 0, 100, 0, 100, 1);
        break;
      case 13:
        renderMenu(soundMenuIndex, "Sound", soundMenuArray, soundMenuLength);
        break;
      case 14:
        renderInfoMenu();
        break;
      default:
        break;
    }
  }
  

  //look for rot switch press
  int presetButton = presetButtonDown();

  if(presetButton != 0){
    if(presetButtonDown() == 4){ //rot button down
      if(!menuBtnWasDown){
  
        if(liveKnobScrollMode){
          //tone(6,2000,50);
        }
        else{
          if(knobMenuIndex == 10 && presetMenuIndex > 0){
            loadPreset(presetMenuIndex); 
            presetMenuIndex = 0;
          }
          if(knobMenuIndex == 11  && presetMenuIndex > 0){
            savePreset(presetMenuIndex); 
            presetMenuIndex = 0; 
          }
        }
  
        liveKnobScrollMode = !liveKnobScrollMode;
  
        if(speedLocked && ((knobMenuIndex >= 3 && knobMenuIndex <= 6) || (knobMenuIndex >= 10 && knobMenuIndex <= 11))){
          liveKnobScrollMode = true;
        }
        
        uView.clear(PAGE);
        firstMenuRun = true;
        myEnc.write(0);
      }
      menuBtnWasDown = true;
    }
    else{
      if(!menuBtnWasDown){ 
        tone(6, 1500, 10);
        delay(160);
        if(presetButtonDown() == presetButton){
          presetButtonAction(presetButton);
          menuBtnWasDown = true;
          uView.clear(PAGE);
          firstMenuRun = true;
          myEnc.write(0);
        }
      }
    }
    
  }
  else{
    menuBtnWasDown = false;
  }
  
}

void presetButtonAction(int presButton){
  if(btnmodeMenuIndex == 0){//preset
    if(!speedLocked){
      loadPreset(presButton);
    }
  }
  if(btnmodeMenuIndex == 1){//speed
    if(!speedLocked){
      int valueCoef = 30;
      speedValue = 100 - (presButton * valueCoef - valueCoef);
      tone(6, 2000, 100);
    }
  }
  if(btnmodeMenuIndex == 2){//rof
    int valueCoef = 25;
    rofValue = 100 - (presButton * valueCoef - valueCoef);
    tone(6, 2000, 100);
  }
  if(btnmodeMenuIndex == 3){//burst
    if(presButton == 3){
      burstMenuIndex = 3;
    }
    else{
      burstMenuIndex = presButton - 1;
    }
    tone(6, 2000, 100);
  }
}





void renderVoltMeter(bool useAlarm){

  //useAlarm = false;
  
  if(firstMenuRun){
    voltMeter->reDraw();
  }

  if(millis() > lastBatteryCheck + batteryCheckDelay){

    float v = (analogRead(voltMeterPin) * vPow) / 1024.0;
    float v2 = v / (r2 / (r1 + r2));

    v2 *= 10;
    int v2Int = (int)v2;
    v2 = (float)v2Int / 10;

    //uView.setCursor(34,0);
    //uView.print(v2);
    //tone(6, 2000, 20);

    batteryCheckSum -= batteryCheck[batteryCheckIndex];
    batteryCheck[batteryCheckIndex] = v2;
    batteryCheckSum += v2;
    batteryCheckIndex++;
    if(batteryCheckIndex >= batteryCheckLength){
      batteryCheckIndex = 0;
    }
    lastBatteryCheck = millis();
    
  }

  float voltLevel = batteryCheckSum / batteryCheckLength;
  voltMeter->setValue(voltLevel * 10);

  if(useAlarm){
    
    if(voltLevel < 11.0){
      if(lastBatAlarm + 3000 < millis()){
        digitalWrite(pusherEnablePin, LOW);
        flywheelESC.writeMicroseconds(1000);
        
        batteryWarning();
        lastBatAlarm = millis();
      }
    }
  
    if(voltLevel < 10.9){
      if(lastBatAlarm + 1500 < millis()){
        digitalWrite(pusherEnablePin, LOW);
        flywheelESC.writeMicroseconds(1000);
        
        batteryWarning();
        lastBatAlarm = millis();
      }
    }
    
    if(voltLevel < 10.8){
      if(lastBatAlarm + 1000 < millis()){
        digitalWrite(pusherEnablePin, LOW);
        flywheelESC.writeMicroseconds(1000);
        
        batteryCritical();
        lastBatAlarm = millis();
      }
    }
  
    if(voltLevel < 10.7){
      while(true){
        digitalWrite(pusherEnablePin, LOW);
        flywheelESC.writeMicroseconds(1000);
        
        batteryCritical();
        delay(200);
      }
    }
  }

  if(!speedLocked){
    uView.setCursor(41,0);
    if(voltLevel < 10){
      uView.print("0");
    }
    uView.print(voltLevel,1);
  }
  
  uView.setCursor(56,39);
  uView.print("B");
    
}

void batteryWarning(){
  tone(6, 2400);
  delay(140);
  tone(6, 4000);
  delay(140);
  tone(6, 1200);
  delay(200);
  noTone(6);
}

void batteryCritical(){
  tone(6, 2400, 140);
  delay(200);
  tone(6, 4000, 140);
  delay(200);
  tone(6, 2400, 140);
  delay(200);
  tone(6, 4000, 140);
}

void renderLockIndicator(){

  uView.setCursor(34,0);
  uView.print("SLOCK");
  uView.setFontType(0);
    
}

void renderInfoMenu(){
  
  uView.setCursor(0,14);
  uView.print("Info");

  uView.setCursor(0,26);
  uView.print("FDL-3");
  uView.setCursor(0,38);
  uView.print("v");
  uView.print(versionNumber,2);
  
  uView.display();
  firstMenuRun = false;
  uView.setFontType(0);
  
}

void renderKnobScrollMenu(){

  encoderChange += myEnc.read();
  myEnc.write(0);
  
  if(abs(encoderChange) >= 8){
    if(!(knobMenuIndex == 0 && encoderChange < 0)){
      knobMenuIndex += encoderChange / 8;//was 4
      knobMenuIndex = constrain(knobMenuIndex, 0, knobMenuLength - 1);
      cacheShots = 0;
    }
    encoderChange = 0;
    if(soundMenuIndex == 0){
      tone(6,2000,10);
    }
  }

  uView.setCursor(0,0);
  uView.print("Mode");

  //uView.setFontType(1);
  int txtWidth = uView.getFontWidth() * (knobMenuArray[knobMenuIndex].length() + 1);
  int txtHeight = uView.getFontHeight();
  int availSpace = 56;
  int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);

  uView.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
  
  uView.setCursor(txtLocX,26);
  uView.print(knobMenuArray[knobMenuIndex]);
  
  uView.display();
  firstMenuRun = false;
  uView.setFontType(0);
  
}

void renderMenu(byte &menuIndex, String label, String menu[], int menuLength){

  encoderChange += myEnc.read();
  myEnc.write(0);
  
  if(abs(encoderChange) >= 4){
    if(!(menuIndex == 0 && encoderChange < 0)){
      menuIndex += encoderChange / 4;
      menuIndex = constrain(menuIndex, 0, menuLength - 1);
    }
    encoderChange = 0;
  }

  uView.setCursor(0,14);
  uView.print(label);

  uView.setFontType(1);

  int txtWidth = uView.getFontWidth() * (menu[menuIndex].length() + 1);
  int txtHeight = uView.getFontHeight();
  int availSpace = 56;
  int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);

  uView.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
  
  uView.setCursor(txtLocX, 26);
  uView.print(menu[menuIndex]);
  
  uView.display();
  firstMenuRun = false;
  uView.setFontType(0);
  
}




void renderGauge(int &gaugeValue, String label, int gaugeMin, int gaugeMax, int valueMin, int valueMax, int detPerMove){

  mainGauge->setMinValue(gaugeMin);
  mainGauge->setMaxValue(gaugeMax);
  
  if(firstMenuRun){
    mainGauge->reDraw();
    //myEnc.write(gaugeValue);
    firstMenuRun = false;
  }

  uView.setCursor(0,4);
  uView.print(label);

  encoderChange += myEnc.read(); //--
  myEnc.write(0); //--

  if(abs(encoderChange) >= detPerMove){
    gaugeValue += encoderChange / detPerMove;
    gaugeValue = constrain(gaugeValue, valueMin, valueMax);
    encoderChange = 0;
  }
  
//  gaugeValue = myEnc.read();
//  if(gaugeValue > valueMax || gaugeValue < valueMin){
//    gaugeValue = constrain(gaugeValue, valueMin, valueMax);
//    myEnc.write(gaugeValue);
//  }

  uView.setCursor(30,40);
  
  uView.print(gaugeValue);
  if(gaugeValue < 10){
    uView.print(" ");
  }
  if(gaugeValue < 100){
    uView.print(" ");
  }

  mainGauge->setValue(gaugeValue);
  uView.display();
}


void renderSplash(){

  uView.setFontType(1);
  uView.setCursor(11,14);
  uView.print("F");
  uView.display();
  delay(50);
  uView.print("D");
  uView.display();
  delay(50);
  uView.print("L");
  uView.display();
  delay(50);
  uView.print("-");
  uView.display();
  delay(50);
  uView.print("3");
  uView.display();

  delay(400);
  
  uView.setFontType(0);
  
  delay(200);
  uView.clear(PAGE);
  uView.display();
  
}

/*
uint8_t bitmapTest [] = {
0x00, 0x00, 0xFC, 0x04, 0x04, 0x04, 0xC4, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x7C, 0x00,
0xFC, 0x04, 0x04, 0x04, 0xC4, 0x44, 0x44, 0x44, 0xC4, 0x84, 0x04, 0x0C, 0x18, 0x30, 0xE0, 0x00,
0x00, 0xFC, 0x04, 0x04, 0x04, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xC7, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x7C, 0x00,
0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xFF, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x10, 0x10, 0x18, 0x06, 0x83, 0xC0, 0x60, 0x30, 0x1F, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xF0, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x80, 0x80, 0xC0, 0x60, 0x20,
0x20, 0x61, 0xC1, 0x81, 0x81, 0x01, 0x01, 0x01, 0xC1, 0xA1, 0x21, 0x21, 0xE1, 0x81, 0x00, 0x00,
0x00, 0x80, 0xE0, 0x20, 0x20, 0xE0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x10, 0x18, 0x0C,
0x84, 0xCC, 0x78, 0x00, 0xC1, 0x79, 0x0F, 0x00, 0x00, 0x07, 0xCC, 0x70, 0x00, 0x83, 0x86, 0x06,
0x43, 0xF1, 0x1C, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x2C, 0x26, 0x22, 0x23,
0x21, 0x20, 0x20, 0x23, 0x23, 0x22, 0x3E, 0x00, 0x3C, 0x23, 0x21, 0x30, 0x1C, 0x07, 0x07, 0x1C,
0x30, 0x23, 0x26, 0x3C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
*/

