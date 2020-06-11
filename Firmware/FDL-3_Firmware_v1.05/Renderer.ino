/////////////
///RENDER////
/////////////
void renderScreen(){
  
  int contrastValue = map(currStSettings.brightness, 0, 100, 0, 255);
  uView.contrast(contrastValue);
  
  renderVoltMeter();
    
  if(liveKnobScrollMode){
    renderKnobScrollMenu();
  }
  else{
    switch(knobMenuIndex){
      case 0:
        renderGauge(currBlSettings.speedValue, "Speed", 0, 100, currStSettings.minSpeed, currStSettings.maxSpeed, 1);
        break;
      case 1:
        renderGauge(currBlSettings.rofValue, "ROF", 0, 100, 0, 100, 1);
        break;
      case 2:
        renderMenu(currBlSettings.burstCount, "Burst", burstMenu, sizeof(burstMenu) / sizeof(size_t));
        break;
      case 3:
        renderGauge(currBlSettings.minSpinup, "MinSpn", 150, 500, 150, 500, 1);
        break;
      case 4:
        renderGauge(currBlSettings.maxSpinup, "MaxSpn", 150, 500, 150, 500, 1);
        break;
      case 5:
        renderMenu(currBlSettings.fireMode, "FireMode", firemodeMenu, sizeof(firemodeMenu) / sizeof(size_t));
        break;  
      case 6:
        renderGauge(currBlSettings.spinDown, "SpnDwn", 6, 25, 6, 25, 8);
        break;
      case 7:
        renderGauge(currBlSettings.idleTime, "Idle", 0, 10, 0, 10, 8);
        break;    
      case 8:
        renderPresetMenu();
        break;
      case 9:
        renderMenu(presetMenuIndex, "Save", presetMenu, sizeof(presetMenu) / sizeof(size_t));
        break;
      case 10:
        renderGauge(currStSettings.minSpeed, "MinSpd", 0, 100, 0, currStSettings.maxSpeed, 1);
        break;
      case 11:
        if(!speedLocked){
          renderGauge(currStSettings.maxSpeed, "MaxSpd", 0, 100, currStSettings.minSpeed, 100, 1);
        }
        break; 
      case 12:
        renderMenu(currStSettings.btnMode, "Btn Mode", btnmodeMenu, sizeof(btnmodeMenu) / sizeof(size_t));
        break;
      case 13:
        renderGauge(currStSettings.brkAgr, "BrkAgr", 3, 25, 3, 25, 8);
        break;
      case 14:
        renderUserLock();
        break;
      case 15:
        renderGauge(currStSettings.brightness, "Bright", 0, 100, 0, 100, 1);
        break;
      case 16:
        renderMenu(currStSettings.soundOn, "Sound", soundMenu, sizeof(soundMenu) / sizeof(size_t));
        break;
      case 17:
        renderGauge(currStSettings.batOffset, "BatOff", -8, 8, -8, 8, 8);
        break;
      case 18:
        renderInfoMenu();
        break;      
      default:
        break;
    }
  }  

  //look for rot switch or preset press
  int presetButton = presetButtonDown();

  if(presetButton != PRSTNONE){
    if(presetButtonDown() == PRSTROT){ //rot button down
      if(!menuBtnWasDown){
                        
        if(liveKnobScrollMode){ //in main menu
          presetMenuIndex = 0;
        }
        else{ //in submenu
          if(knobMenu[knobMenuIndex] == "Load" && presetMenuIndex > 0){
            loadPreset(presetMenuIndex); 
          }
          if(knobMenu[knobMenuIndex] == "Save"  && presetMenuIndex > 0){
            writePreset(presetMenuIndex); 
          }
        }

        liveKnobScrollMode = !liveKnobScrollMode; //flip between main menu and setting
  
        if(speedLocked && knobMenu[knobMenuIndex] == "MaxSpd"){ //ignore flip if locked
          liveKnobScrollMode = true;
        }
        
        uView.clear(PAGE);
        firstMenuRun = true;
        myEnc.write(0);
      }
      menuBtnWasDown = true;
    }
    else{ //preset button pressed   
      if(!menuBtnWasDown){ 
        toneAlt(1500, 10);
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
  if(currStSettings.btnMode == 0){//preset
    loadPreset(presButton);
  }
  if(currStSettings.btnMode == 1){//speed
    int valueCoef = 30;
    currBlSettings.speedValue = 100 - (presButton * valueCoef - valueCoef);
    toneAlt(2000, 100);
  }
  if(currStSettings.btnMode == 2){//rof
    int valueCoef = 25;
    currBlSettings.rofValue = 100 - (presButton * valueCoef - valueCoef);
    toneAlt(2000, 100);
  }
  if(currStSettings.btnMode == 3){//burst
    if(presButton == 3){
      currBlSettings.burstCount = 3;
    }
    else{
      currBlSettings.burstCount = presButton - 1;
    }
    toneAlt(2000, 100);
  }
}
