#define HW_PIN 15
#define SP_rPIN 35
#define SP_tPIN 25
#define SWITCH_PIN 38
#define LED_PIN 4
#define LED_ON LOW
#define LED_OFF HIGH

#include "serial.h"

SerialConfiguration HW, SP;

#include "hobbywing.h"
#include "smart_port.h"


void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  
  pinMode(SWITCH_PIN, INPUT);

  HW=SerialConfigure(19200, false, HW_PIN, -1);
  SP=SerialConfigure(57600, true,  SP_rPIN, SP_tPIN);

  Serial.begin(2000000);

  //pinMode(32, OUTPUT);
  //pinMode(25, OUTPUT);
  //digitalWrite(32, HIGH);
  //digitalWrite(25, HIGH);
}

int i=0;
unsigned long m[1000];

void loop() {

  //Measure Pin States
  /*SerialSynchronize(SP);
  for(i=1;i<1000;i++) m[i]=MeasurePinState(SP, i%2);
  for(i=1;i<1000;i++){
    Serial.print("Stan: "); Serial.print(i%2); Serial.print(" Długość: "); Serial.println(m[i]);
  }*/


  //Get bytes
  /*SerialSynchronize(SP);
  for(i=1;i<1000;i++) m[i]=SerialReceiveByte(&SP);
  for(i=1;i<1000;i++){
    if(m[i]==0x7e) Serial.println();
    Serial.print(m[i], HEX); Serial.print(" ");
  }*/







  digitalWrite(LED_PIN, LED_ON);
  SerialSynchronize(HW);
  digitalWrite(LED_PIN, LED_OFF);

  while(1){
    SerialReceiveByte(&HW);

    //0x9B   0x9B  0x03  0xE8  0x01  0x08  0x5B  0x00  0x01  0x00  0x21  0x21  0xB9
    if(HW_IsID()){
      for(int i=1;i<=13;i++)HW.B[SerialBufSize-i]=0;      
    }else if(HW_IsData()){
      HW_PrintData(true);
      for(int i=1;i<=19;i++)HW.B[SerialBufSize-i]=0;
      break;
    }
  }

  digitalWrite(LED_PIN, LED_ON);
  SerialSynchronize(SP);
  digitalWrite(LED_PIN, LED_OFF);

  int i=0;
  while(1){
    SerialReceiveByte(&SP);

    
    if(SP.B[SerialBufSize-2]==0x7e && SP.B[SerialBufSize-1]==0x00){
      if(i==0) SP_Send(SP, FRSKY_SP_A3, HW_Data.Voltage*100.0);
      if(i==1) SP_Send(SP, FRSKY_SP_CURR, HW_Data.Current*100.0);
      if(i==2) SP_Send(SP, FRSKY_SP_RPM, HW_Data.RPM);
      if(i==3) SP_Send(SP, FRSKY_SP_T1, HW_Data.T_FET);
      if(i==4) SP_Send(SP, FRSKY_SP_T2, HW_Data.T);
      if(i==5) break;

      i++;
    }
  }
  
}
