const int frontDownLoopDelay = 3; //delay till pass front switch if start from front rare case
const int frontDownLoopDur = 50; //wait delay till front switch up error if not met
const int frontUpLoopDur = 200; //wait delay to hit front switch error if not hit
const int frontPassDelay = 5; //pseudo debounce delay at front switch
const int rearUpLoopDur = 80; //first wait for rear switch
const int rearUpLoopDur2 = 200; //backup wait for rear switch
const int pusherBlipDur = 8; //pusher blip in attempts to reach back switch

void fireSequence(){

    if(currBlSettings.fireMode == SAFETY){
      toneAlt(3000, 50);
      delay(100);
      toneAlt(2000, 100);
      return true;
    }
    
    //start wheels 
    int spinupDelay = getSpinupByMotorSpeed();
    currentSpeed = readESCPower();
    setFwSpeed(currentSpeed);
    
    unsigned long loopLastCheck = 0;

    //get cached shot count  
    byte cacheShots = getBurstCount();
    bool triggerState = true;
    
    //spinup
    loopLastCheck = millis();
    while(millis() - loopLastCheck < spinupDelay ){
      triggerLogic(triggerState, cacheShots);
    }

    //if in abide and trigger up, shutdown
    if(!triggerDown() && currBlSettings.fireMode == ABIDE){
      cycleShutdown();
      idleRelease = millis() + currBlSettings.idleTime * 1000;
      spindownTarget = MINTHROTTLE;      
      return true;
    }

    //shots cached
    while(enterFireLoop(cacheShots)){

      //disable brake
      digitalWrite(pusherBrakePin, LOW); 
      delayMicroseconds(200);

      digitalWrite(pusherEnablePin, HIGH);      
      
      //fire cycle begin
      //while front down / left at front, run
      //rare state
      loopLastCheck = millis();
      while(pusherSwitchDown() == PSHFRNT){
        if(millis() - loopLastCheck < frontDownLoopDur){          
          triggerLogic(triggerState, cacheShots);
          delay(frontDownLoopDelay);
        }
        else{
          break;
        }          
      }
      
      //run till front hit
      loopLastCheck = millis();
      logicSwitchCheck = millis();
      hitCount = 0;

      while(!checkCount()){
        if(millis() - loopLastCheck < frontUpLoopDur){
          triggerLogic(triggerState, cacheShots);
        }
        else{
          //front not reached in time, shutdown and beep
          cycleShutdown();
          toneAlt(2000, 100);
          return true;
        }
      }
      
      if(enterAutoLoop(cacheShots)){ //more than one shot remaining

        //short delay to get past front switch
        loopLastCheck = millis();
        while(millis() - loopLastCheck < frontPassDelay){ }
      
        //get delay between shots
        int burstBreakDelay = map(currBlSettings.rofValue, 0, 100, 100, 0);

        //blip brakes
        if(burstBreakDelay > 5){
            digitalWrite(pusherEnablePin, LOW);
            delayMicroseconds(200);
            digitalWrite(pusherBrakePin, HIGH);

            loopLastCheck = millis();
            while(millis() - loopLastCheck < burstBreakDelay){ }
            
            digitalWrite(pusherBrakePin, LOW);
            delayMicroseconds(200);
        }

        digitalWrite(pusherEnablePin, HIGH);
        cacheShots--;
      }
      else{ //final shot
        cacheShots--;
        break;        
      }      
    }

    //cycle finish
    //shut down pusher
    digitalWrite(pusherEnablePin, LOW);         
        
    //blip brakes and coast
    loopLastCheck = millis();        
    delayMicroseconds(200);      
    digitalWrite(pusherBrakePin, HIGH);
    while(pusherSwitchDown() != PSHREAR && millis() - loopLastCheck < currStSettings.brkAgr){ }
    digitalWrite(pusherBrakePin, LOW);            
    delayMicroseconds(200);    
    
    //while rear not down, coast till hit
    //blip if fail, attempt once to correct
    loopLastCheck = millis();    
    while(pusherSwitchDown() != PSHREAR){
      if(millis() - loopLastCheck < rearUpLoopDur){ }
      else{        
        //back not hit attempt pusher blip to get it there
        digitalWrite(pusherEnablePin, HIGH);
        delay(pusherBlipDur);
        digitalWrite(pusherEnablePin, LOW);  
        
        //while rear not down, coast till hit
        loopLastCheck = millis();
        while(pusherSwitchDown() != PSHREAR){
          if(millis() - loopLastCheck < rearUpLoopDur2){ }
          else{
            //rear not found, beep and shutdown cycle
            toneAlt(3000, 80);
            delay(80);
            toneAlt(3000, 80);
            cycleShutdown();
            return true;
          }
        }
      }
    }   

    //hard stop at rear
    delayMicroseconds(200);    
    digitalWrite(pusherBrakePin, HIGH);
    
    //set brake release for 200ms from now
    brakeRelease = millis() + 200;    
    spindownTarget = MINTHROTTLE;
    cycleShutdown();

    //spindown until trigger released
    while(triggerDown() && !inFullAuto()){ 
      spinDownFW(false);
    };

    idleRelease = millis() + currBlSettings.idleTime * 1000 + 100;
}
