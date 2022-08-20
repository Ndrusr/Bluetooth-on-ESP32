#include <Arduino.h>


class ResistSensor{
public:

  ResistSensor(int chargePin, int pin, int dividerRes, float calibrate[4] ):
    chargePin(chargePin), measurePin(pin), divider(dividerRes), a(calibrate[0]), b(calibrate[1]), c(calibrate[2]), d(calibrate[3])
    {
      pinMode(chargePin, OUTPUT);
      pinMode(pin, INPUT);
      digitalWrite(chargePin, LOW);

    }

  void ReadSensor(){
    timeNow = micros();
    float err = 100;
    // do{
    digitalWrite(chargePin, HIGH);
    //delayMicroseconds(250);
    refV = (analogRead(measurePin));
    
    //Serial.println(refV);
    measuredRes = (divider)*((4095.0/refV)-1);
    digitalWrite(chargePin, LOW);
    
    timeLag = micros() - timeNow;
    // }while(abs(err)*100 > 5);
  }  int getRes(){
    return measuredRes;
  }
  float getForce(){
    return (b-log((measuredRes-d)/c))/a;
  }
  float getHertz(){
    return 1/(timeLag/1e6);
  }
protected:
  float a;
  float b;
  float c;
  float d;
  int divider;
  int measuredRes;
  int measurePin;
  int chargePin;
  float refV;
  int timeNow;
  int timeLag;
  int prev{0};
  
};