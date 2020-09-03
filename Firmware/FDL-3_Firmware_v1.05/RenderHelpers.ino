
void renderVoltMeter(){  
  if(firstMenuRun){
    voltMeter->reDraw();
  }

  float voltLevel = getVoltLevel();
  voltMeter->setValue(voltLevel * 10);

  if(speedLocked){
    renderLockIndicator();
  }
  else{
    uView.setCursor(41,0);
    if(voltLevel < 10){
      uView.print("0");
    }
    uView.print(voltLevel,1);    
  }
   
  if(voltLevel < 10.8){
    if(lastBatAlarm + 3000 < millis()){
      shutdownFwAndPush();
      batteryWarning();
      lastBatAlarm = millis();
    }
  }

  if(voltLevel < 10.7){
    if(lastBatAlarm + 1500 < millis()){
      shutdownFwAndPush();
      batteryWarning();
      lastBatAlarm = millis();
    }
  }
  
  if(voltLevel < 10.6){
    if(lastBatAlarm + 1000 < millis()){
      shutdownFwAndPush();
      batteryCritical();
      lastBatAlarm = millis();
    }
  }

  if(voltLevel < 10.5){
    while(true){
      shutdownFwAndPush();
      batteryCritical();
      delay(200);
    }
  }
  
  uView.setCursor(56,39);
  uView.print("B");    
}

void batteryWarning(){
  toneAlt(2400, 140);
  toneAlt(4000, 140);
  toneAlt(1200, 200);
}

void batteryCritical(){
  toneAlt(2400, 140);
  delay(100);
  toneAlt(4000, 140);
  delay(100);
  toneAlt(2400, 140);
  delay(100);
  toneAlt(4000, 140);
}

void renderLockIndicator(){
  int xLoc = 34;
  if(currStSettings.maxSpeed < 100){ xLoc = 40; }
  if(currStSettings.maxSpeed < 10){ xLoc = 46; }  
  uView.setCursor(xLoc,0);
  uView.print("SL");
  uView.print(currStSettings.maxSpeed);
  uView.setFontType(0);    
}

void renderInfoMenu(){  
  uView.setCursor(0,14);
  uView.print("Info");
  uView.setCursor(0,26);
  uView.print("FDL-3");
  uView.setCursor(0,38);
  uView.print("v");
  float versionNum = (float)versionNumber / 100;
  uView.print(versionNum,2);  
  uView.display();
  firstMenuRun = false;
  uView.setFontType(0);  
}

void renderUserLock(){

  while(presetButtonDown() == PRSTROT){}
  
  uView.clear(PAGE);  
  uView.setCursor(6,0);
  uView.print("User Lock");
  
  if(currStSettings.usrLock == 0){
    uView.setCursor(9,20);
    uView.print("Set");
  }
  else{
    uView.setCursor(3,20);
    uView.print("Enter");
  }
  uView.print(" Code");
  uView.display();

  if(currStSettings.usrLock == 0){
    toneAlt(2400, 80);
    toneAlt(1600, 80);
    toneAlt(2400, 140);
  }
  else{
    toneAlt(1600, 140);
    toneAlt(1200, 140);
    delay(200);
  }
  
  int usrLock = 0;
  
  for(int i = 0; i < 3; i++){
    int prstBut = 0;
    while(prstBut == 0){ prstBut = presetButtonDown(); }
    while(presetButtonDown() != 0) { };
    toneAlt(2400, 120);    
    if(prstBut == 4){ break; }

    if(i > 0 && i < 3){ usrLock *= 10; }
    usrLock += prstBut;

    if(usrLock > 0){
      uView.setCursor(0,36);  
      uView.print(usrLock);
      uView.display();
    }
  }
  delay(300);
  
  usrLock = usrLock < 100 ? 0 : usrLock;

  if(usrLock > 100){ //code fully entered
    if(currStSettings.usrLock == 0){
      currStSettings.usrLock = usrLock;
      writeStaticSettings();

      uView.clear(PAGE);
      uView.setCursor(6,18);
      uView.print("Code Set");
      uView.setCursor(0,36);
      uView.print(currStSettings.usrLock);
      uView.display();      
      
      toneAlt(1600, 80);
      delay(50);
      toneAlt(2400, 140);
      delay(300);
    }
    else{
      if(currStSettings.usrLock == usrLock){
        currStSettings.usrLock = 0;
        writeStaticSettings();

        uView.clear(PAGE);
        uView.setCursor(8,16);
        uView.print("Unlocked");
        uView.display();        
        
        toneAlt(2400, 80);
        delay(30);
        toneAlt(2400, 80);
        delay(30);
        toneAlt(3000, 80);
        delay(100);

        knobMenuIndex = 0;
        liveKnobScrollMode = false;
        firstMenuRun = true;
      }
    }
  }
  else{
    if(currStSettings.usrLock == 0){
      liveKnobScrollMode = true;      
    }
    toneAlt(1600, 80);
    delay(200);
  }

  uView.clear(PAGE);
  uView.display();  
}

/////////////////
//main menu
/////////////////
void renderKnobScrollMenu(){
  encoderChange += myEnc.read();
  myEnc.write(0);

  byte arraySize = sizeof(knobMenu) / sizeof(size_t);
  if(abs(encoderChange) >= 8){
    if(!(knobMenuIndex == 0 && encoderChange < 0)){
      knobMenuIndex += encoderChange / 8;
      knobMenuIndex = constrain(knobMenuIndex, 0, arraySize - 1);
    }
    encoderChange = 0;    
    toneAlt(2000,10);
  }

  uView.setCursor(0,0);
  uView.print("Mode");

  int textLength = 0;
  const char* testPtr = knobMenu[knobMenuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  int txtWidth = uView.getFontWidth() * (textLength + 1);
  int txtHeight = uView.getFontHeight();
  int availSpace = 56;
  int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);

  uView.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
  
  uView.setCursor(txtLocX,26);
  testPtr = knobMenu[knobMenuIndex];
  while(*testPtr != '\0'){
    uView.print(*testPtr);
    testPtr++;
  }
  
  uView.display();
  firstMenuRun = false;
  uView.setFontType(0);  
}

/////////////////
//menu type settings
/////////////////
void renderMenu(byte &menuIndex, const char label[], const char* menu[], byte arraySize){
  encoderChange += myEnc.read();
  myEnc.write(0);

  if(abs(encoderChange) >= 4){
    if(!(menuIndex == 0 && encoderChange < 0)){
      menuIndex += encoderChange / 4;
      menuIndex = constrain(menuIndex, 0, arraySize - 1);
    }
    encoderChange = 0;
  }

  uView.setCursor(0,14);
  uView.print(label);

  uView.setFontType(1);

  int textLength = 0;
  const char* testPtr = menu[menuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  int txtWidth = uView.getFontWidth() * (textLength + 1);
  int txtHeight = uView.getFontHeight();
  int availSpace = 56;
  int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);

  uView.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
  
  uView.setCursor(txtLocX, 26);
  testPtr = menu[menuIndex];
  while(*testPtr != '\0'){
    uView.print(*testPtr);
    testPtr++;
  } 
  
  uView.display();
  firstMenuRun = false;
  uView.setFontType(0);  
}


/////////////////
//gauge type settings
/////////////////
void renderGauge(int &gaugeValue, String label, int gaugeMin, int gaugeMax, int valueMin, int valueMax, int detPerMove){
  mainGauge->setMinValue(gaugeMin);
  mainGauge->setMaxValue(gaugeMax);
  
  if(firstMenuRun){
    mainGauge->reDraw();
    firstMenuRun = false;
  }

  uView.setCursor(0,4);
  uView.print(label);
    
  encoderChange += myEnc.read();
  myEnc.write(0);

  if(abs(encoderChange) >= detPerMove || firstMenuRun){
    gaugeValue += encoderChange / detPerMove;
    gaugeValue = constrain(gaugeValue, valueMin, valueMax);
    encoderChange = 0;
  }

  uView.setCursor(30,40);    
  uView.print(gaugeValue);
  if(gaugeValue < 10){ uView.print(" "); }
  if(gaugeValue < 100){ uView.print(" "); }

  mainGauge->setValue(gaugeValue);
  uView.display();
}


void renderSplash(String splashText){
  uView.setFontType(1);

  int spaceIndex = splashText.indexOf(' ');

  if(spaceIndex == -1){
    int availSpace = 60;
    int txtWidth = uView.getFontWidth() * splashText.length();
    int txtLocX = constrain((availSpace - txtWidth) / 2, 0, availSpace / 2);
  
    uView.setCursor(txtLocX,16);
  
    for(int i = 0; i < splashText.length(); i++){
      uView.print(splashText[i]);
      uView.display();
      delay(50);
    }
  }
  else{
    String sub1 = splashText.substring(0, spaceIndex);
    String sub2 = splashText.substring(spaceIndex + 1);

    int availSpace = 60;

    if(sub1.length() > 7){
      uView.setFontType(0);
    }
    else{
      uView.setFontType(1);
    }
    
    int txtWidth1 = uView.getFontWidth() * sub1.length();        
    int txtLocX1 = constrain((availSpace - txtWidth1) / 2, 0, availSpace / 2);    
  
    uView.setCursor(txtLocX1,10);
  
    for(int i = 0; i < sub1.length(); i++){
      uView.print(sub1[i]);
      uView.display();
      delay(50);
    }

    if(sub2.length() > 7){
      uView.setFontType(0);
    }
    else{
      uView.setFontType(1);
    }
    
    int txtWidth2 = uView.getFontWidth() * sub2.length();
    int txtLocX2 = constrain((availSpace - txtWidth2) / 2, 0, availSpace / 2);

    uView.setCursor(txtLocX2,24);
  
    for(int i = 0; i < sub2.length(); i++){
      uView.print(sub2[i]);
      uView.display();
      delay(50);
    }
  }

  delay(600);  
  uView.setFontType(0);  
  uView.clear(PAGE);
  uView.display();  
}

void renderPresetMenu(){
  encoderChange += myEnc.read();
  myEnc.write(0);

  byte arraySize = sizeof(presetMenu) / sizeof(size_t);
  if(abs(encoderChange) >= 4){
    if(!(presetMenuIndex == 0 && encoderChange < 0)){
      presetMenuIndex += encoderChange / 4;
      presetMenuIndex = constrain(presetMenuIndex, 0, arraySize - 1);
    }
    encoderChange = 0;
  }

  uView.clear(PAGE);
  if(presetMenuIndex > 0){
    renderPreset(presetMenuIndex);
  }
  else{
    firstMenuRun = true;
    if(speedLocked){ renderLockIndicator(); }  
    renderVoltMeter();
    
    uView.setCursor(0,14);
    uView.print("Load");
  
    uView.setFontType(1);

    int textLength = 0;
    const char* testPtr = presetMenu[presetMenuIndex];
    while(*(testPtr++) != '\0'){
      textLength++;
    }
    int txtWidth = uView.getFontWidth() * (textLength + 1);
    int txtHeight = uView.getFontHeight();
    int availSpace = 56;
    int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);
  
    uView.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
    
    uView.setCursor(txtLocX, 26);    
    testPtr = presetMenu[presetMenuIndex];
    while(*testPtr != '\0'){
      uView.print(*testPtr);
      testPtr++;
    } 
  }
  
  uView.display();
  firstMenuRun = false;
  uView.setFontType(0);  
}

void renderPreset(byte preset){
  readPreset(preset);

  uView.setFontType(0);    
  uView.setCursor(0,0);
  uView.print(firemodeMenu[readBlSettings.fireMode]);  
  uView.setCursor(0,13);
  uView.print("S:");
  uView.print(readBlSettings.speedValue);  
  uView.setCursor(0,26);
  uView.print("B:");
  uView.print(burstMenu[readBlSettings.burstCount]);  
  uView.setCursor(0,39);
  uView.print("R:");
  uView.print(readBlSettings.rofValue);  

  uView.setCursor(32,0);
  uView.print("p:");
  uView.print(readBlSettings.minSpinup);
  uView.setCursor(32,13);
  uView.print("P:");
  uView.print(readBlSettings.maxSpinup);
  uView.setCursor(32,26);
  uView.print("D:");
  uView.print(readBlSettings.spinDown);
  uView.setCursor(32,39);
  uView.print("I:");
  uView.print(readBlSettings.idleTime);

  uView.rectFill(57, 39, 8, 9, WHITE , NORM);
  uView.setColor(BLACK);
  uView.setCursor(58,40);
  uView.print(preset);
  uView.setColor(WHITE);
}
