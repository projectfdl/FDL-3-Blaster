#define PSHFRNT 1
#define PSHREAR 2
#define PSHNONE 0

#define PRSTNONE 0
#define PRST1 1
#define PRST2 2
#define PRST3 3
#define PRSTROT 4

#define SAFETY 0
#define ABIDE 1
#define AUTO 2
#define CACHED 3

unsigned long logicLastCheck = 0;
unsigned long logicSwitchCheck = 0;
int clickCount = 0;
int hitCount = 0;

void cycleShutdown(){
  digitalWrite(pusherEnablePin, LOW);
  spinDownFW(true);
}

void shutdownFwAndPush(){
  digitalWrite(pusherEnablePin, LOW);
  setFwSpeed(NOTHROTTLE);
}

void spinDownFW(bool resetCheckTimer){
    if(resetCheckTimer){
      lastSpinCheck = 0;
    }
  
    if(currentSpeed > spindownTarget){
      if(currBlSettings.spinDown == 0){
        currentSpeed = spindownTarget;
      }
      else{
        unsigned long elapsedTime = lastSpinCheck == 0 ? 0 : millis() - lastSpinCheck;
        lastSpinCheck = millis();  
        int spindown = elapsedTime / currBlSettings.spinDown * 10;
        lastSpinCheck -= elapsedTime % currBlSettings.spinDown;        
        currentSpeed -= spindown;
        currentSpeed = max(spindownTarget, currentSpeed);
      }
    }
    setFwSpeed(currentSpeed);
}

bool checkCount(){  
  if(millis() - logicSwitchCheck > 1 && pusherSwitchDown() == PSHFRNT){
    logicSwitchCheck = millis();
    hitCount++;
  }

  if(hitCount > 3){    
    hitCount = 0;    
    return true;
  }
  else{
    return false;
  }
}

void triggerLogic(bool &triggerState, byte &shotsCached){  
  if(currBlSettings.fireMode == CACHED){ // in cache mode
      
    if(triggerState == false && triggerDown()){
      clickCount++;
    }
    if(triggerState == true && !triggerDown()){
      clickCount++;
    }

    if(clickCount > 50){
      clickCount = 0;
      triggerState = !triggerState;
      if(triggerState == true){
        shotsCached++;
      }
    }    
  }  
}

bool enterFireLoop(byte shotsCached){
  if(shotsCached > 0){  
    if(inFullAuto()){//full auto always abide by trigger except in auto mode for single shot
      return triggerDown() || (currBlSettings.fireMode != ABIDE && shotsCached >= 100);
    }  
    if(currBlSettings.fireMode == ABIDE){//trigger abide mode
      return triggerDown();
    }
    if(currBlSettings.fireMode == AUTO){//auto mode
      return true;
    }
    if(currBlSettings.fireMode == CACHED){//cached mode
      return true;
    }
  }
  
  return false;
}

bool enterAutoLoop(byte shotsCached){  
  if(shotsCached > 1){    
    if(inFullAuto()){//full auto always abide by trigger
      return triggerDown();
    }
    if(currBlSettings.fireMode == ABIDE){//trigger abide mode
      return triggerDown();
    }
    if(currBlSettings.fireMode == AUTO){//auto mode
      return true;
    }
    if(currBlSettings.fireMode == CACHED){//cached mode
      return true;
    }
  }
  
  return false;
}

bool inFullAuto(){
  return getBurstCount() == 100;
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
    return PRSTNONE;
  }
  if(readVal < 700){
    return PRSTROT;
  }
  if(readVal < 800){
    return PRST3;
  }
  if(readVal < 900){
    return PRST2;
  }
  return PRST1;
}

int pusherSwitchDown(){
  int readVal = analogRead(pusherSwitchPin);

  if(readVal < 400){
    return PSHNONE;
  }
  if(readVal < 900){
    return PSHFRNT;
  }
  return PSHREAR;
}

int readESCPower(){
  return map(currBlSettings.speedValue, 0, 100, MINTHROTTLE, MAXTHROTTLE);
}

void setFwSpeed(int newSpeed){
  flywheelESC.writeMicroseconds(newSpeed);
}

int getSpinupByMotorSpeed(){   
  double calcSpin = doubleMap(currBlSettings.speedValue, 0, 100, currBlSettings.minSpinup, currBlSettings.maxSpinup);
  if(currentSpeed < MINTHROTTLE){ return calcSpin; }
  if(currBlSettings.speedValue == 0){ return 0; }
  
  double minSync = 50;
  double targetThrottle = doubleMap(currBlSettings.speedValue, 0, 100, MINTHROTTLE, MAXTHROTTLE);  
  double throttleFactor =  (targetThrottle - currentSpeed) / (targetThrottle - MINTHROTTLE);
  calcSpin = calcSpin * throttleFactor - minSync;
  
  return max(0, calcSpin);
}

double doubleMap(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

byte getBurstCount(){
  byte arraySize = sizeof(burstMenu) / sizeof(size_t);
  if(currBlSettings.burstCount >= arraySize - 1){
    return 100;
  }
  else{
    return currBlSettings.burstCount + 1;
  }
}

int getROF(){
  return currBlSettings.rofValue;
}
