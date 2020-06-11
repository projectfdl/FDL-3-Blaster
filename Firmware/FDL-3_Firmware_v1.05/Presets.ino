byte offset = 0;
byte writes = 0;

void loadSettings(){
  byte readVersion = EEPROM.read(0);
  
  if(readVersion != versionNumber){
    //try again
    delay(200);
    readVersion = EEPROM.read(1);
    
    if(readVersion != versionNumber){    
      writeDefaultSettings();
    }
  }
  else{
    loadStaticSettings();
    loadCurrentSettings();
  }
}

void loadStaticSettings(){
  EEPROM.get(8, currStSettings);
  lastStSettings = currStSettings;
}

void writeStaticSettings(){
  if(!staticSettingsChanged()){ return; }
  
  EEPROM.put(8, currStSettings);
  lastStSettings = currStSettings;
}

bool staticSettingsChanged(){
  return (
    currStSettings.btnMode != lastStSettings.btnMode ||
    currStSettings.brightness != lastStSettings.brightness ||
    currStSettings.soundOn != lastStSettings.soundOn ||
    currStSettings.batOffset != lastStSettings.batOffset ||
    currStSettings.brkAgr != lastStSettings.brkAgr ||
    currStSettings.minSpeed != lastStSettings.minSpeed ||
    currStSettings.maxSpeed != lastStSettings.maxSpeed ||    
    currStSettings.usrLock != lastStSettings.usrLock );
}

void loadCurrentSettings(){
  offset = EEPROM.read(4);
  int settingsLoc = 102 + offset * 20;
  EEPROM.get(settingsLoc, currBlSettings);
  lastBlSettings = currBlSettings;
}

void writeCurrentSettings(){
  if(!currentSettingsChanged()){ return; }
  
  int writesLoc = 100 + offset * 20;
  EEPROM.get(writesLoc, writes);
  writes += 1;
  
  if(writes > 200){
    offset += 1;
    if(offset > 40){ offset = 0; }
    writesLoc = 100 + offset * 20;    
    writes = 0;
    EEPROM.put(4, offset);  
  }
  EEPROM.put(writesLoc, writes);
  
  int settingsLoc = 102 + offset * 20;
  EEPROM.put(settingsLoc, currBlSettings);
  lastBlSettings = currBlSettings;
}

bool currentSettingsChanged(){
  return (
    currBlSettings.speedValue != lastBlSettings.speedValue ||
    currBlSettings.rofValue != lastBlSettings.rofValue ||
    currBlSettings.burstCount != lastBlSettings.burstCount ||
    currBlSettings.minSpinup != lastBlSettings.minSpinup ||
    currBlSettings.maxSpinup != lastBlSettings.maxSpinup ||
    currBlSettings.fireMode != lastBlSettings.fireMode ||
    currBlSettings.spinDown != lastBlSettings.spinDown ||
    currBlSettings.idleTime != currBlSettings.idleTime);
}

void readPreset(byte presetIndex){
  int presetLoc = 24 + (presetIndex - 1) * 20;
  EEPROM.get(presetLoc, readBlSettings);
}

void writePreset(byte presetIndex){
  int presetLoc = 24 + (presetIndex - 1) * 20;
  EEPROM.put(presetLoc, currBlSettings); 
  
  toneAlt(2000, 50);
  delay(50);
  toneAlt(2000, 100);   
}

void loadPreset(byte presetIndex){
  int presetLoc = 24 + (presetIndex - 1) * 20;
  EEPROM.get(presetLoc, currBlSettings); 
  lastBlSettings = currBlSettings;

  toneAlt(4000, 50);
  delay(50);
  toneAlt(4000, 100);
}

void writeDefaultSettings(){  
  uView.clear(PAGE);
  uView.setCursor(0,0);
  uView.print("Updating");
  uView.setCursor(0,14);
  uView.print("settings");
  uView.setCursor(0,28);
  float versionNum = (float)versionNumber / 100;
  uView.print(versionNum,2);
  uView.display();  
  
  //write defaults to all 
  EEPROM.put(0, versionNumber);
  EEPROM.put(1, versionNumber);
  offset = 0;
  EEPROM.put(4, offset);
  EEPROM.put(8, defStSettings);
  writes = 0;
  EEPROM.put(100, writes);
  EEPROM.put(102, defBlSettings); 
  EEPROM.put(24, defBlSettings);
  EEPROM.put(44, defBlSettings);
  EEPROM.put(64, defBlSettings); 
  currStSettings = defStSettings;
  lastStSettings = defStSettings;
  currBlSettings = defBlSettings;
  lastBlSettings = defBlSettings;  

  delay(1000);
  toneAlt(3000, 50);
  delay(50);
  toneAlt(2500, 100);
  delay(50);
  toneAlt(2000, 200);
  delay(100);
  
  uView.clear(PAGE);
  uView.display();
}
