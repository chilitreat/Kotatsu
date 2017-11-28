#include "esp32-hal-ledc.h"
#include <Nefry.h>
#include <NefryIFTTT.h>
#include <NefryFireBase.h>

#define PWM_BITWIDTH 16

String Event, SecretKey, FirebaseId;
String currentState = "off";
NefryFireBase firebase;

void setup() {
  Nefry.setStoreTitle("SecretKey",0); //Nefry DataStoreのタイトルを指定
  Nefry.setStoreTitle("Event",1);     //Nefry DataStoreのタイトルを指定
  Nefry.setStoreTitle("FirebaseId",2);     //Nefry DataStoreのタイトルを指定
  SecretKey = Nefry.getStoreStr(0);   //Nefry DataStoreからデータを取得
  Event = Nefry.getStoreStr(1);       //Nefry DataStoreからデータを取得
  FirebaseId = Nefry.getStoreStr(2);       //Nefry DataStoreからデータを取得
  
  ledcSetup(1, 50, PWM_BITWIDTH); // channel 1, 50 Hz, 16-bit depth
  ledcAttachPin(A0, 1);   // GPIO 22 on channel 1
   
  firebase.begin(FirebaseId);
  Serial.begin(115200);
  Nefry.enableSW();                   //SWを有効化
}
 
void loop() {
  String isOn = firebase.read("Kotatsu/Power");

  Nefry.println(isOn);

  if(isOn.indexOf("on") != -1 && currentState.indexOf("off") != -1){
//    Nefry.println("点灯点灯点灯！！！！！！！！！！！！！！！！！！！！！！！");
    turnOn();
  }else if(isOn.indexOf("off") != -1 && currentState.indexOf("on") != -1){
//    Nefry.println("消灯消灯消灯！！！！！！！！！！！！！！！！！！！！！！！");
    turnOff();
  }

  currentState = isOn;
  
  if (Nefry.readSW()) {
    DataElement elem = DataElement();
    elem.setValue("Power", isOn.indexOf("off")!=-1 ? "on" : "off");
    firebase.update("Kotatsu", &elem);
  }  
  
  Nefry.ndelay(1000 * 10); //30分休む。本当は365日休みたい
}

void turnOn(){
  ledcWrite(1, deg2pw(80, PWM_BITWIDTH));
  Notify(true);
}

void turnOff(){
  ledcWrite(1, deg2pw(120, PWM_BITWIDTH));
  Notify(false);
}

void Notify(bool isOn){
  String value1 = "";

  if(isOn){
      value1 = "⚠⚠⚠現在こたつがついています⚠⚠⚠";
  }else{
      value1 = "現在こたつはついていません。安心してお出かけください。";
  }
  
  if (!IFTTT.send(Event, SecretKey, value1)) {//IFTTTにデータを送信
      Nefry.setLed(255, 0, 0);        //Errの時、赤色点灯
  }
}

// 角度をパルス幅に変換
int deg2pw(int deg, int bit){
    double ms = ((double) deg - 90.0) * 0.95 / 90.0 + 1.45;
    return (int) (ms / 20.0 * pow(2, bit));
}


