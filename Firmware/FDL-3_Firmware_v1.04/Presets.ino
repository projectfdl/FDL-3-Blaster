
void boundSettings(){
  speedValue = constrain(speedValue, 0, 100);
  rofValue = constrain(rofValue, 0, 100);
  brightnessValue = constrain(brightnessValue, 0, 100);

  minSpeedValue = constrain(minSpeedValue, 0, 100);
  maxSpeedValue = constrain(maxSpeedValue, 0, 100);
  minSpinupValue = constrain(minSpinupValue, 120, 500);
  maxSpinupValue = constrain(brightnessValue, 150, 500);
  brightnessValue = constrain(brightnessValue, 0, 100);

  if(spindownRate < 0){ spindownRate = 12; }
  spindownRate = constrain(spindownRate, 0, 12);  
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
      spindownRate
    };

    int memLoc = presetNumber * 100;
      
    EEPROM.put(memLoc, currentSettings);

    //tone(6,3000,50);
    
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
