const int frontDownLoopDelay = 3;
const int frontDownLoopDur = 50;
const int frontUpLoopDur = 200;
const int frontPassDelay = 5;
const int brakeBlipDur = 7;
const int rearUpLoopDur = 80;
const int rearUpLoopDur2 = 200;
const int pusherBlipDur = 8;
const int brakeHardDur = 30;

bool fireSequence(){

    if(firemodeMenuIndex == SAFETY){
      tone(6, 3000, 50);
      delay(100);
      tone(6, 2000, 100);
      return true;
    }

    //get cached shot count
    if(inFullAuto() || firemodeMenuIndex != CACHED){ 
      cacheShots = 0; 
    }    
    cacheShots += getBurstCount();
    

    //start wheels
    currentSpeed = readESCPower();
    flywheelESC.writeMicroseconds(currentSpeed);

    int spinupDelay = getSpinup();  
    triggerState = true;
    
    unsigned long loopLastCheck = 0;
    int loopDuration = 0;

    //spinup
    loopLastCheck = millis();
    loopDuration = spinupDelay;
    while(millis() - loopLastCheck < loopDuration ){
      triggerLogic();
    }

    //if in abide and trigger up, shutdown
    if(!triggerDown() && firemodeMenuIndex == ABIDE){
      cycleShutdown(true);      
      return true;
    }

    //shots cached
    while(enterFireLoop()){

      digitalWrite(pusherEnablePin, HIGH);
      
      loopLastCheck = millis();
      loopDuration = frontDownLoopDur;

      //fire cycle begin
      //while front down / left at front, run
      //rare state
      while(pusherSwitchDown() == PSHFRNT){
        if(millis() - loopLastCheck < loopDuration){          
          triggerLogic();
          delay(frontDownLoopDelay);
        }
        else{
          break;
        }          
      }

      loopLastCheck = millis();
      loopDuration = frontUpLoopDur;
      
      //run till front hit
      logicSwitchCheck = millis();
      hitCount = 0;

      while(!checkCount()){
          if(millis() - loopLastCheck < loopDuration){
            triggerLogic();
          }
          else{
            //front not reached in time, shutdown and beep
            cycleShutdown(true);
            tone(6,2000,100);
            return true;
          }
      }
      
      if(enterAutoLoop()){ //more than one shot remaining

        //short delay to get past front switch
        loopLastCheck = millis();
        loopDuration = frontPassDelay;
        while(millis() - loopLastCheck < loopDuration){
          triggerLogic();
        }
      
        //get delay between shots
        int burstBreakDelay = map(rofValue, 0, 100, 100, 0);

        //blip brakes
        if(burstBreakDelay > 5){
            digitalWrite(pusherEnablePin, LOW);
            delayMicroseconds(200);
            digitalWrite(pusherBrakePin, HIGH);

            loopLastCheck = millis();
            while(millis() - loopLastCheck < burstBreakDelay){
              triggerLogic();
            }
            
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
    loopDuration = brakeBlipDur;
        
    delayMicroseconds(200);      
    digitalWrite(pusherBrakePin, HIGH);
    while(pusherSwitchDown() != PSHREAR && millis() - loopLastCheck < loopDuration){
      triggerLogic();
    }
    digitalWrite(pusherBrakePin, LOW);            
    delayMicroseconds(200);
    
    
    //while rear not down, coast till hit
    //blip if fail, attempt once to correct
    loopLastCheck = millis();
    loopDuration = rearUpLoopDur;
    
    while(pusherSwitchDown() != PSHREAR){
      if(millis() - loopLastCheck < loopDuration){  
        triggerLogic();      
      }
      else{        
        //back not hit attempt pusher blip to get it there
        digitalWrite(pusherEnablePin, HIGH);
        delay(pusherBlipDur);
        digitalWrite(pusherEnablePin, LOW);
  
        loopLastCheck = millis();
        loopDuration = rearUpLoopDur2;
        //while rear not down, coast till hit
        while(pusherSwitchDown() != PSHREAR){
          if(millis() - loopLastCheck < loopDuration){
            triggerLogic();           
          }
          else{
            //rear not found, beep and shutdown cycle
            tone(6,3000,80);
            delay(80);
            tone(6,3000,80);
            cycleShutdown(true);
            return true;
          }
        }
      }
    }     


    //hard stop at rear
    delayMicroseconds(200);    
    digitalWrite(pusherBrakePin, HIGH);

    loopLastCheck = millis();
    loopDuration = brakeHardDur;
    
    while(millis() - loopLastCheck < loopDuration){
      triggerLogic();
    }
    digitalWrite(pusherBrakePin, LOW);           
    delayMicroseconds(200);

    cycleShutdown(inFullAuto() || firemodeMenuIndex != CACHED);

    //spindown until trigger released
    while(triggerDown() && !inFullAuto()){ 
      triggerLogic();
      spinDown();
    };
    
    return true;
}
