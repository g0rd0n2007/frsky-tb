//HobbyWing
//19 200 bps
//52us for bit


typedef struct{
  float RxThr, OutPWM;
  float Voltage;
  float Current;
  unsigned int RPM;
  float T_FET, T;
}HW_tData;

HW_tData HW_Data;

bool HW_IsID(){
  return HW.B[SerialBufSize-13]==0x9b && HW.B[SerialBufSize-12]==0x9b && HW.B[SerialBufSize-11]==0x03 && HW.B[SerialBufSize-1]==0xb9;
}

bool HW_IsData(){
  return HW.B[SerialBufSize-19]==0x9b;
}

long HW_GetID(){
  return (HW.B[SerialBufSize-18]<<16) | (HW.B[SerialBufSize-17]<<8) | HW.B[SerialBufSize-16];
}

float HW_GetRxThr(){
  return ((HW.B[SerialBufSize-15]<<8) | HW.B[SerialBufSize-14])/10.23;
}

float HW_GetOutPWM(){
  return ((HW.B[SerialBufSize-13]<<8) | HW.B[SerialBufSize-12])/10.23;
}

unsigned long HW_GetRPM(){
  return ((HW.B[SerialBufSize-11]<<16) | (HW.B[SerialBufSize-10]<<8) | (HW.B[SerialBufSize-9]));
}

float HW_GetVoltage(){
  return ((HW.B[SerialBufSize-8]<<8) | HW.B[SerialBufSize-7])/114.0;
}

float HW_GetCurrent(){
  return ((HW.B[SerialBufSize-6]<<8) | HW.B[SerialBufSize-5])/1.0;
}

float HW_GetT_FET(){
  return ((HW.B[SerialBufSize-4]<<8) | HW.B[SerialBufSize-3])*0.01;
}

float HW_GetT(){
  return ((HW.B[SerialBufSize-2]<<8) | HW.B[SerialBufSize-1])*0.01;
}

void HW_PrintData(bool deb){
  HW_Data.RxThr=HW_GetRxThr();
  HW_Data.OutPWM=HW_GetOutPWM();
  HW_Data.RPM=HW_GetRPM();
  HW_Data.Voltage=HW_GetVoltage();
  HW_Data.Current=HW_GetCurrent();
  HW_Data.T_FET=HW_GetT_FET();
  HW_Data.T=HW_GetT();

  if(deb){
    Serial.print("RxThr:"); Serial.print(HW_GetRxThr()); Serial.print(",");
    Serial.print("OutPWM:"); Serial.print(HW_GetOutPWM()); Serial.print(",");
    Serial.print("RPM:"); Serial.print(HW_GetRPM()); Serial.print(",");
    Serial.print("Voltage:"); Serial.print(HW_Data.Voltage); Serial.print(",");
    Serial.print("Current:"); Serial.print(HW_Data.Current); Serial.print(",");
    Serial.print("T_FET:"); Serial.print(HW_GetT_FET()); Serial.print(",");
    Serial.print("T:"); Serial.print(HW_GetT());
    Serial.println();
  }
}
