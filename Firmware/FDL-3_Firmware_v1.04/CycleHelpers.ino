#define PSHFRNT 1
#define PSHREAR 2
#define PSHNONE 0

unsigned long logicLastCheck = 0;
bool triggerState = false;
int clickCount = 0;

unsigned long logicSwitchCheck = 0;
int hitCount = 0;


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

void cycleShutdown(bool clearCache){
  if(clearCache){
    cacheShots = 0;
  }
  digitalWrite(pusherEnablePin, LOW);
  digitalWrite(pusherBrakePin, LOW);
  lastTriggerUp = millis();
  elapsedTime = 0;
  lastSpindownCheck = millis();
  spinDown();
}

void triggerLogic(){
  
  if(firemodeMenuIndex == CACHED){ // in cache mode
      
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
        cacheShots++;
      }
    }
    
  }
  
}


bool enterFireLoop(){

  if(cacheShots > 0){  
    if(inFullAuto()){//full auto always abide by trigger except in auto mode for single shot
      return triggerDown() || (firemodeMenuIndex != ABIDE && cacheShots >= 100);
    }  
    if(firemodeMenuIndex == ABIDE){//trigger abide mode
      return triggerDown();
    }
    if(firemodeMenuIndex == AUTO){//auto mode
      return true;
    }
    if(firemodeMenuIndex == CACHED){//cached mode
      return true;
    }
  }
  
  return false;
}

bool enterAutoLoop(){
  
  if(cacheShots > 1){    
    if(inFullAuto()){//full auto always abide by trigger
      return triggerDown() || firemodeMenuIndex == CACHED;
    }
    if(firemodeMenuIndex == ABIDE){//trigger abide mode
      return triggerDown();
    }
    if(firemodeMenuIndex == AUTO){//auto mode
      return true;
    }
    if(firemodeMenuIndex == CACHED){//cached mode
      return true;
    }
  }
  
  return false;
}

bool inFullAuto(){
  return getBurstCount() == 100;
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
