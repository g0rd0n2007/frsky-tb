//HobbyWing
//19 200 bps
//52us for bit

#define ESCHW4_NTC_BETA 3950.0
#define ESCHW4_NTC_R1 10000.0
#define ESCHW4_NTC_R_REF 47000.0
#define ESCHW4_DIFFAMP_GAIN 10 //13.6
#define ESCHW4_DIFFAMP_SHUNT 0.25 / 1000
#define ESCHW4_V_REF 3.3
#define ESCHW4_ADC_RES 4096.0


typedef struct{
  float RxThr, OutPWM;
  float Voltage;
  float Current;
  unsigned int RPM;
  float T_FET, T;
}HW_tData;

HW_tData HW_Data;
uint16_t HW_c0 = 0;

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
  //return ((HW.B[SerialBufSize-8]<<8) | HW.B[SerialBufSize-7])/114.0;
  
  uint16_t v = (HW.B[SerialBufSize-8]<<8) | HW.B[SerialBufSize-7];

  return ESCHW4_V_REF * v / ESCHW4_ADC_RES * 11;
}

float HW_GetCurrent(){
  //return ((HW.B[SerialBufSize-6]<<8) | HW.B[SerialBufSize-5])/1.0;

  uint16_t HW_c = (HW.B[SerialBufSize-6]<<8) | HW.B[SerialBufSize-5];
  
  if(HW_Data.RPM==0) HW_c0 = HW_c;

  if (HW_c - HW_c0 < 0) return 0;
  return (HW_c - HW_c0) * ESCHW4_V_REF / (ESCHW4_DIFFAMP_GAIN * ESCHW4_DIFFAMP_SHUNT * ESCHW4_ADC_RES);
}

float CalcTemp(uint16_t t){
  float voltage = t * ESCHW4_V_REF / ESCHW4_ADC_RES;
  float ntcR_Rref = (voltage * ESCHW4_NTC_R1 / (ESCHW4_V_REF - voltage)) / ESCHW4_NTC_R_REF;
  
  if (ntcR_Rref < 0.001) return 0;
  
  float temperature = 1 / (log(ntcR_Rref) / ESCHW4_NTC_BETA + 1 / 298.15) - 273.15;
  if (temperature < 0) return 0;
  return temperature;
}

float HW_GetT_FET(){
  return CalcTemp((HW.B[SerialBufSize-4]<<8) | HW.B[SerialBufSize-3]);
}

float HW_GetT(){
  return CalcTemp((HW.B[SerialBufSize-2]<<8) | HW.B[SerialBufSize-1]);
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
