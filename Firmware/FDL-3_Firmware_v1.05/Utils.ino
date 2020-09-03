
//roll custom tone generator to avoid conflict with Timer2 and abstract pin
void toneAlt(int frequency, int duration){

  if(!currStSettings.soundOn){ return; }
  
  int freqDelay = 1000000 / frequency / 2;
  int loopCount = duration * 1000 / (freqDelay * 2);

  unsigned long offTime = millis() + duration;
  while(millis() < offTime){
    digitalWrite(buzzerPin, HIGH); 
    delayMicroseconds(freqDelay); 
    digitalWrite(buzzerPin, LOW); 
    delayMicroseconds(freqDelay);
  }
}

void initBatteryCheck(){
  //init battery average read values
  for(int x = 0; x < batteryCheckLength; x++){
    batteryCheck[x] = 12.0;
  }
  lastBatteryCheck = millis();
  batteryCheckSum = 12.0 * batteryCheckLength;
}

float getVoltLevel(){
  
 if(millis() > lastBatteryCheck + 300){ //check every 300ms
    const float vPow = 5.042;;
    const float r1 = 100000;
    const float r2 = 10000;

    float v = (analogRead(voltMeterPin) * vPow) / 1024.0;
    float v2 = v / (r2 / (r1 + r2));

    v2 *= 10;
    int v2Int = (int)v2;
    v2 = (float)v2Int / 10;

    batteryCheckSum -= batteryCheck[batteryCheckIndex];
    batteryCheck[batteryCheckIndex] = v2;
    batteryCheckSum += v2;
    batteryCheckIndex++;
    if(batteryCheckIndex >= batteryCheckLength){
      batteryCheckIndex = 0;
    }
    lastBatteryCheck = millis();    
  }
  
  return batteryCheckSum / batteryCheckLength + (float)currStSettings.batOffset / 10.0;
}

void startUpBeeps(){
  toneAlt(2000, 60);
  delay(60);
  toneAlt(2000, 60);
  delay(60);
  toneAlt(2000, 60);
}

void clearSetRoutine(){
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
      writeDefaultSettings();
      break;
    }
  }
  uView.clear(PAGE);
  uView.display();
}

void clearLockRoutine(){
  while(lockOn() && presetButtonDown() == 3){ delay(50); };
  if(presetButtonDown() != 3){ return; }
  while(presetButtonDown() == 3){ delay(50); };
  while(presetButtonDown() == 0){ delay(50); };
  if(presetButtonDown() != 2){ return; }

  currStSettings.usrLock = 0;
  writeStaticSettings();

  uView.clear(PAGE);  
  uView.setCursor(10,16);
  uView.print("Unlocked");
  uView.display();        
  
  toneAlt(2400, 80);
  delay(30);
  toneAlt(2400, 80);
  delay(30);
  toneAlt(3000, 80);
  delay(100);

  uView.clear(PAGE); 
  uView.display(); 
}
