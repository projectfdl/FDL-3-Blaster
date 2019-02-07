
#include <Servo.h>
int escPin = A3;
int speedPin = A2;
int limiterPin = A1;
int triggerPin = A0;

Servo flywheelESC;

int currentSpeed = 1000;
int spindownRate = 10; //units per 10ms
unsigned long lastSpindownCheck = 0;
unsigned long elapsedTime = 0;

void setup() {
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(speedPin, INPUT);
  flywheelESC.attach(escPin); 
  flywheelESC.writeMicroseconds(1000);
}

// the loop function runs over and over again forever
void loop() {
 if(triggerDown()){

    int readSpeed = analogRead(speedPin);
    int limSpeed = analogRead(limiterPin);
    int maxSpeed = map(limSpeed, 0, 1023, 1285, 2000);
    int txSpeed = map(readSpeed, 0, 1023, 1285, maxSpeed);
  
    flywheelESC.writeMicroseconds(txSpeed);//1750);
    currentSpeed = txSpeed;
    elapsedTime = 0;
    lastSpindownCheck = millis();
  }
  else{
    //flywheelESC.writeMicroseconds(1000);
    spinDown();
  }

  //delay(2);
}

void spinDown(){

    int flipSpindown = (16 - spindownRate) * 2;
  
    if(currentSpeed > 1000){
      if(spindownRate == 0){
        currentSpeed = 1000;
      }
      else{
        elapsedTime = lastSpindownCheck == 0 ? 0 : elapsedTime + millis() - lastSpindownCheck;
        lastSpindownCheck = millis();
        int spindown = elapsedTime / 10 * flipSpindown;
        elapsedTime %= 10;
        
        currentSpeed -= spindown;
      }
    }
    
    flywheelESC.writeMicroseconds(currentSpeed);
}

boolean triggerDown(){
  return digitalRead(triggerPin) == LOW;
}
