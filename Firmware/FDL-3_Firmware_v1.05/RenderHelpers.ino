
void renderVoltMeter(){  
  if(firstMenuRun){
    //voltMeter->reDraw();
    // PARTY
  }

  float voltLevel = getVoltLevel();
  //voltMeter->setValue(voltLevel * 10);
  // PARTY

  if(speedLocked){
    renderLockIndicator();
  }
  else{
    oled.setCursor(41,0);
    if(voltLevel < 10){
      oled.print("0");
    }
    oled.print(voltLevel,1);    
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
  
  oled.setCursor(56,39);
  oled.print("B");    
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
  oled.setCursor(xLoc,0);
  oled.print("SL");
  oled.print(currStSettings.maxSpeed);
  //PARTY
  oled.setFont(0);    
}

void renderInfoMenu(){  
  oled.setCursor(0,14);
  oled.print("Info");
  oled.setCursor(0,26);
  oled.print("FDL-3");
  oled.setCursor(0,38);
  oled.print("v");
  float versionNum = (float)versionNumber / 100;
  oled.print(versionNum,2);  
  oled.display();
  firstMenuRun = false;
  //PARTY
  oled.setFont(0);  
}

void renderUserLock(){

  while(presetButtonDown() == PRSTROT){}
  
  oled.clearDisplay();  
  oled.setCursor(6,0);
  oled.print("User Lock");
  
  if(currStSettings.usrLock == 0){
    oled.setCursor(9,20);
    oled.print("Set");
  }
  else{
    oled.setCursor(3,20);
    oled.print("Enter");
  }
  oled.print(" Code");
  oled.display();

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
      oled.setCursor(0,36);  
      oled.print(usrLock);
      oled.display();
    }
  }
  delay(300);
  
  usrLock = usrLock < 100 ? 0 : usrLock;

  if(usrLock > 100){ //code fully entered
    if(currStSettings.usrLock == 0){
      currStSettings.usrLock = usrLock;
      writeStaticSettings();

      oled.clearDisplay();
      oled.setCursor(6,18);
      oled.print("Code Set");
      oled.setCursor(0,36);
      oled.print(currStSettings.usrLock);
      oled.display();      
      
      toneAlt(1600, 80);
      delay(50);
      toneAlt(2400, 140);
      delay(300);
    }
    else{
      if(currStSettings.usrLock == usrLock){
        currStSettings.usrLock = 0;
        writeStaticSettings();

        oled.clearDisplay();
        oled.setCursor(8,16);
        oled.print("Unlocked");
        oled.display();        
        
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

  oled.clearDisplay();
  oled.display();  
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

  oled.setCursor(0,0);
  oled.print("Mode");

  int textLength = 0;
  const char* testPtr = knobMenu[knobMenuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  // PARTY
  // PROBLEM!
  //int txtWidth = oled.getFontWidth() * (textLength + 1);
  //int txtHeight = oled.getFontHeight();
  int txtWidth = 5 * (textLength + 1);
  int txtHeight = 7;
  int availSpace = 56;
  int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);

  //PARTY oled.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
  
  oled.setCursor(txtLocX,26);
  testPtr = knobMenu[knobMenuIndex];
  while(*testPtr != '\0'){
    oled.print(*testPtr);
    testPtr++;
  }
  
  oled.display();
  firstMenuRun = false;
  //PARTY
  oled.setFont(0);  
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

  oled.setCursor(0,14);
  oled.print(label);

//PARTY
  oled.setFont(1);

  int textLength = 0;
  const char* testPtr = menu[menuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  //int txtWidth = oled.getFontWidth() * (textLength + 1);
  //int txtHeight = oled.getFontHeight();
  int txtWidth = 8 * (textLength + 1);
  int txtHeight = 16;
  int availSpace = 56;
  int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);

  //PARTY oled.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
  
  oled.setCursor(txtLocX, 26);
  testPtr = menu[menuIndex];
  while(*testPtr != '\0'){
    oled.print(*testPtr);
    testPtr++;
  } 
  
  oled.display();
  firstMenuRun = false;
  //PARTY
  oled.setFont(0);  
}


/////////////////
//gauge type settings
/////////////////
void renderGauge(int &gaugeValue, String label, int gaugeMin, int gaugeMax, int valueMin, int valueMax, int detPerMove){
  //mainGauge->setMinValue(gaugeMin);
  //mainGauge->setMaxValue(gaugeMax);
  //PARTY 
  
  if(firstMenuRun){
//PARTY     mainGauge->reDraw();
    firstMenuRun = false;
  }

  oled.setCursor(0,4);
  oled.print(label);
    
  encoderChange += myEnc.read();
  myEnc.write(0);

  if(abs(encoderChange) >= detPerMove || firstMenuRun){
    gaugeValue += encoderChange / detPerMove;
    gaugeValue = constrain(gaugeValue, valueMin, valueMax);
    encoderChange = 0;
  }

  oled.setCursor(30,40);    
  oled.print(gaugeValue);
  if(gaugeValue < 10){ oled.print(" "); }
  if(gaugeValue < 100){ oled.print(" "); }

//PARTY   mainGauge->setValue(gaugeValue);
  oled.display();
}


void renderSplash(String splashText){

  oled.setTextSize(LARGE_T);

  int spaceIndex = splashText.indexOf(' ');

  if(spaceIndex == -1){
    int txtWidth = CH_LW * (splashText.length() + 1) ;
    int txtLocX = constrain((OLED_WIDTH - txtWidth) / 2, 0, OLED_WIDTH / 2);
    int txtLocY = constrain((OLED_HEIGHT - CH_LH) / 2, 0, OLED_HEIGHT / 2);
  
    oled.setCursor(txtLocX,txtLocY);
  
    for(int i = 0; i < splashText.length(); i++){
      oled.print(splashText[i]);
      oled.display();
      delay(50);
    }
  }
  else{
    String sub1 = splashText.substring(0, spaceIndex);
    String sub2 = splashText.substring(spaceIndex + 1);

    int charWidth = CH_LW;
    int charHeight = CH_LH;

    if(sub1.length() * CH_LW > OLED_WIDTH){
      oled.setTextSize(SMALL_T);
      charWidth = CH_SW;
      charHeight = CH_SH;
    }
    else{
      oled.setTextSize(LARGE_T);
    }
    
    int txtWidth1 = charWidth * (sub1.length() + 1);        
    int txtLocX1 = constrain((OLED_WIDTH - txtWidth1) / 2, 0, OLED_WIDTH / 2);    
    int txtLocY1 = constrain((OLED_HEIGHT - charHeight) / 2, 0, OLED_HEIGHT / 2);
    txtLocY1 -= (charHeight/2 + 1);

    oled.setCursor(txtLocX1,txtLocY1);
  
    for(int i = 0; i < sub1.length(); i++){
      oled.print(sub1[i]);
      oled.display();
      delay(50);
    }
    charWidth = 5;
    if(sub2.length() * CH_LW > OLED_WIDTH){
      oled.setTextSize(SMALL_T);
      charWidth = CH_SW;
      charHeight = CH_SH;
    }
    else{
      oled.setTextSize(LARGE_T);
      charWidth = CH_LW;
      charHeight = CH_LH;
    }

    int txtWidth2 = charWidth * (sub2.length()+1);
    int txtLocX2 = constrain((OLED_WIDTH - txtWidth2) / 2, 0, OLED_WIDTH / 2);    
    int txtLocY2 = constrain((OLED_HEIGHT - charHeight) / 2, 0, OLED_HEIGHT / 2);
    txtLocY2 += charHeight/2 + 1;

    oled.setCursor(txtLocX2,txtLocY2);
  
    for(int i = 0; i < sub2.length(); i++){
      oled.print(sub2[i]);
      oled.display();
      delay(50);
    }
  }

  delay(600);  
  oled.clearDisplay();
  oled.display();  
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

  oled.clearDisplay();
  if(presetMenuIndex > 0){
    renderPreset(presetMenuIndex);
  }
  else{
    firstMenuRun = true;
    if(speedLocked){ renderLockIndicator(); }  
    renderVoltMeter();
    
    oled.setCursor(0,14);
    oled.print("Load");

  //PARTY
    oled.setFont(1);

    int textLength = 0;
    const char* testPtr = presetMenu[presetMenuIndex];
    while(*(testPtr++) != '\0'){
      textLength++;
    }
    //PARTY
    //int txtWidth = oled.getFontWidth() * (textLength + 1);
    //int txtHeight = oled.getFontHeight();
    int txtWidth = 7 * (textLength + 1);
    int txtHeight = 16;
    int availSpace = 56;
    int txtLocX = constrain((availSpace - txtWidth) / 2 - 1, 0, availSpace / 2);
  
    //PARTY oled.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
    
    oled.setCursor(txtLocX, 26);    
    testPtr = presetMenu[presetMenuIndex];
    while(*testPtr != '\0'){
      oled.print(*testPtr);
      testPtr++;
    } 
  }
  
  oled.display();
  firstMenuRun = false;
  //PARTY
  oled.setFont(0);  
}

void renderPreset(byte preset){
  readPreset(preset);

  //PARTY
  oled.setFont(0);    
  oled.setCursor(0,0);
  oled.print(firemodeMenu[readBlSettings.fireMode]);  
  oled.setCursor(0,13);
  oled.print("S:");
  oled.print(readBlSettings.speedValue);  
  oled.setCursor(0,26);
  oled.print("B:");
  oled.print(burstMenu[readBlSettings.burstCount]);  
  oled.setCursor(0,39);
  oled.print("R:");
  oled.print(readBlSettings.rofValue);  

  oled.setCursor(32,0);
  oled.print("p:");
  oled.print(readBlSettings.minSpinup);
  oled.setCursor(32,13);
  oled.print("P:");
  oled.print(readBlSettings.maxSpinup);
  oled.setCursor(32,26);
  oled.print("D:");
  oled.print(readBlSettings.spinDown);
  oled.setCursor(32,39);
  oled.print("I:");
  oled.print(readBlSettings.idleTime);

  //PARTY oled.rectFill(57, 39, 8, 9, WHITE , NORM);
//PARTY   oled.setColor(BLACK);
  oled.setCursor(58,40);
  oled.print(preset);
//PARTY   oled.setColor(WHITE);
}
