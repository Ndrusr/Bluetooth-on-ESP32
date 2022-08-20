#define DEBUG
#include <Arduino.h>
#include"sensor.h"
#include<vector>
// #include<NimBLEDevice.h>
#include<WiFi.h>
#include<Firebase_ESP_Client.h>
#include<iostream>
#include<string>

#include"addons/TokenHelper.h"
#include"addons/RTDBHelper.h"

#define WIFISSID "SUTD_LAB"

#define APIKEY "AIzaSyD2TU0UxpJpOuW2OBsXVqzXEbkohpD3k2M"
#define DBURL "https://the-pressure-is-killing-me-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define MEASPIN 36
#define CHARGEPIN1 14
#define CHARGEPIN2 25
#define CHARGEPIN3 32
#define RES 100.0F
#define LEDPIN1 21
#define LEDPIN2 18
#define LEDPIN3 4

FirebaseData fbdo;

FirebaseAuth fbAuth;
FirebaseConfig fbCfg;

bool signupOK{false};
unsigned long sendDataPrevMillis{0};

int MODE = 0;

float sensor2Calib[4] = {0.15333936823711974 , 1.5193198324068933 , 62.51617758380474 , 170.09574665570986};
float sensor3Calib[4] = {0.08285024543286705 , 1.3985125496097506 , 47.73275905875787 , 211.07650096510048};
float sensor1Calib[4] = {0.047888027937037746 , 1.973877204271394 , 27.69486075050948 , 177.17698606093919};

ResistSensor sensor1(CHARGEPIN1, MEASPIN, RES, sensor1Calib);
ResistSensor sensor2(CHARGEPIN2, MEASPIN, RES, sensor2Calib);
ResistSensor sensor3(CHARGEPIN3, MEASPIN, RES, sensor3Calib);

// #define SERVICE_UUID        "459c0c10-0eb9-11ed-861d-0242ac120002"
// #define CHARACTERISTIC_UUID "2c0b7b90-0eba-11ed-861d-0242ac120002"
//vector

int mmHg = 15;

int timeNow;


// class customBLECallback : public BLECharacteristicCallbacks
// {
// public:
// customBLECallback(){

// }
// void onRead(BLECharacteristic* pchar){
//   Serial.println("Read request recieved");
// }

// void onWrite(BLECharacteristic* pchar){
//   int temp = pchar->getValue<int>();
//   mmHg = temp;
//   Serial.print("New pressure value written: ");
//   Serial.println(mmHg);
// }

// };


void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  #endif
  WiFi.begin(WIFISSID);
  while(WiFi.status() != WL_CONNECTED){
    #ifdef DEBUG
    Serial.print(".");
    sleep(1);
    #endif
  }
  #ifdef DEBUG
  Serial.println("\n\rConnected!");
  
  #endif
  fbCfg.api_key = APIKEY;
  fbCfg.database_url = DBURL;


  if(Firebase.signUp(&fbCfg, &fbAuth,"","")){
    #ifdef DEBUG
    Serial.println("Connected to DB");
    #endif
    signupOK = true;
  }else{
    #ifdef DEBUG
    Serial.printf("%s\n", fbCfg.signer.signupError.message.c_str());
    #endif
  }

  fbCfg.token_status_callback = tokenStatusCallback;

  Firebase.begin(&fbCfg, &fbAuth);
  Firebase.reconnectWiFi(true);
  // Serial.println("BLE Services Begin.");
  // BLEDevice::init("ESP32 WAP");
  // BLEServer *pserver = BLEDevice::createServer();
  // BLEService *pservice = pserver->createService(SERVICE_UUID);
  // BLECharacteristic *pcharacteristic = pservice->createCharacteristic(CHARACTERISTIC_UUID, 
  //                                                                     NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::WRITE);
  
  // Serial.print("characteristic val length: ");
  // Serial.println(pcharacteristic->getDataLength());

  // pcharacteristic->setValue(mmHg);

  // Serial.print("characteristic val length: ");
  // Serial.println(pcharacteristic->getDataLength());

  // pservice->start();
  // BLEAdvertising *padvertising = BLEDevice::getAdvertising();
  // padvertising->addServiceUUID(SERVICE_UUID);
  // padvertising->setScanResponse(true);
  // padvertising->setMinPreferred(0x06);
  // padvertising->setMaxPreferred(0x12);

  // pcharacteristic->setCallbacks(new customBLECallback);

  // BLEDevice::startAdvertising();
  pinMode(LEDPIN1, OUTPUT);
  pinMode(LEDPIN2, OUTPUT);
  pinMode(LEDPIN3, OUTPUT);
  digitalWrite(LEDPIN1, HIGH);
  digitalWrite(LEDPIN2, HIGH);
  digitalWrite(LEDPIN3, HIGH);
  delay(50);
  digitalWrite(LEDPIN1, LOW);
  digitalWrite(LEDPIN2, LOW);
  digitalWrite(LEDPIN3, LOW);
}

void loop() {
  // if(Serial.available()){
  //   BlueSer.write(Serial.read());
  // }
  // timeNow = micros();
  #ifdef DEBUG
  if(Serial.available() >= 1){
    try{
      MODE = (Serial.readString()).toInt();
    }catch(...){
      Serial.println("read failed");
    }
  }
  #endif

  if(Firebase.ready() && signupOK && (millis()-sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    if(Firebase.RTDB.getInt(&fbdo, "/pressure/Sock1")){
      if(fbdo.dataType() == "int"){
        if(fbdo.intData() != mmHg){
          mmHg = fbdo.intData();
          #ifdef DEBUG
          Serial.println(mmHg);
          #endif
        }
        
      }
    }else{
      #ifdef DEBUG
      Serial.println(fbdo.errorReason());
      #endif
    }
  }
  sensor1.ReadSensor();
  #ifdef DEBUG
  
  // delay(500);
  #endif

  sensor2.ReadSensor();
  #ifdef DEBUG
  if(MODE != 0){
    if(MODE == 1){
      Serial.print("sensor1: ");
      //Serial.print(sensor1.getRes());
      Serial.println(sensor1.getForce());
    }else if(MODE == 2){
      Serial.print("sensor2: ");
      //Serial.print(sensor2.getRes());
      Serial.println(sensor2.getForce());
    }else{
      Serial.print("sensor3: ");
      //Serial.print(sensor3.getRes());
      Serial.println(sensor3.getForce());
    }
    delay(500);
  }
  
  
  #endif 
  
  sensor3.ReadSensor();
  #ifdef DEBUG

  // delay(500);
  #endif
  // Serial.print(sensor1.getRes());
  // Serial.print(" Ohm, Clock: ");
  // //Serial.println(mmHg);
  if(sensor1.getForce() > mmHg - 2 && sensor1.getForce() < mmHg + 2){
    digitalWrite(LEDPIN1, HIGH);
  }else{
    digitalWrite(LEDPIN1, LOW);
  }
  if(sensor2.getForce() > mmHg + 1  && sensor2.getForce() < mmHg + 3){
    digitalWrite(LEDPIN2, HIGH);
  }else{
    digitalWrite(LEDPIN2, LOW);
  }
  if(sensor3.getForce() > mmHg + 4 && sensor3.getForce() < mmHg + 6){
    digitalWrite(LEDPIN3, HIGH);
  }else{
    digitalWrite(LEDPIN3, LOW);
  }
  // if(sensor1.getRes()<101 && sensor1.getRes()>96){
  //   Serial.println("Within threshold");
  // }else{
  //   Serial.println("Exceeded threshold");
  // }
  // delayMicroseconds((int32_t)(1e6/HERTZ));
  delay(500);
  
}