/////////////
///RENDER////
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
          renderGauge(minSpinupValue, "Spn Min", 120, 500, 120, 500, 1); //250, 500, 250, 500);
        }
        break;
      case 4:
        if(!speedLocked){
          renderGauge(maxSpinupValue, "Spn Max", 150, 500, 150, 500, 1);
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


