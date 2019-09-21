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


void renderSplash(String splashText){

  uView.setFontType(1);

  int spaceIndex = splashText.indexOf(' ');

  if(spaceIndex == -1){
    int availSpace = 60;
    int txtWidth = uView.getFontWidth() * splashText.length();
    //int txtHeight = uView.getFontHeight();    
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

  delay(400);
  
  uView.setFontType(0);
  
  delay(200);
  uView.clear(PAGE);
  uView.display();
  
}
