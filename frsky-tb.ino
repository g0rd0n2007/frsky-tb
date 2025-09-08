#include <Arduino.h>
#include <HardwareSerial.h>
#include "BluetoothSerial.h"
#include "hobbywing.h"

const char* TB_Version = "0.1.3";

HardwareSerial escSerial(1);
HardwareSerial frskySerial(2);
BluetoothSerial bluetoothSerial;

const int escRX_PIN = 32;
const int frskyRX_PIN = 33;
const int frskyTX_PIN = 14;
const int LED_PIN = 2;

int HobbywingSignatureIdx = -1;
float Thr = 0.0;
float OutPWM = 0.0;
float Voltage = 0.0;
float Temp1 = 0.0;
float Temp2 = 0.0;
uint32_t RPM = 0;
float Current = 0.0;
uint16_t CurrentOffset = 0;

#define MAX_FRAME_SIZE 20
uint8_t escFrame[MAX_FRAME_SIZE];
uint8_t escFrameIndex = 0;
uint8_t frskyFrame[MAX_FRAME_SIZE];
uint8_t frskyFrameIndex = 0;

//Free FrSky sensor ID that are not connected to line
/*uint8_t freeIDs[255] = {0};
uint8_t freeIDidx = 0;*/

#define U16(i) ((buf[i] << 8) | buf[i+1])
#define U24(i) ((buf[i] << 16) | (buf[i+1] << 8) | (buf[i+2]))
#define U32(i) ((buf[i] << 24) | (buf[i+1] << 16) | (buf[i+2] << 8) | (buf[i+3]))



void printFrame(const char* str, uint8_t* buf, uint8_t len) {
  Serial.printf("%s ", str);
  for(int i=0; i<len; i++) Serial.printf("%02X ", buf[i]);
  Serial.println();
}

/*bool idIsFree(uint16_t id){
  for(int i=0; i<freeIDidx;i++){
    if(freeIDs[i]==id)return false;
  }
  return true;
}*/


float calcTemp(uint16_t temp_raw){
  const float Vref = 3.3f;
  const float Rfixed = 283000.0f;//2800.0f;//200000.0f;   // << dobrany ~200 kΩ
  const float R0 = 10000.0f;        // NTC 10k @25°C
  const float B  = 3950.0f;

  float voltage    = (temp_raw / 4095.0f) * Vref;
  float resistance = Rfixed * voltage / (Vref - voltage);
  //float resistance = Rfixed * (Vref - voltage) / voltage;
  float kelvin     = 1.0f / ((1.0f/298.15f) + (1.0f/B)*log(resistance/R0));
  return kelvin - 273.15f; // °C
}



// Funkcja dekodująca dane telemetryczne
void decodeHwFrame(uint8_t* buf, uint8_t len) {
  if (len < 10) return; // minimalna długość ramki

  //printFrame("[HW Frame]", buf, len);

  uint32_t frame_id = U24(1);
  uint16_t rx_thr = U16(4);
  Thr = rx_thr / 10.0;
  uint16_t out_pwm_raw = U16(6);
  OutPWM = out_pwm_raw / 10.0;

  uint32_t rpm_raw = U24(8);
  RPM = rpm_raw;

  uint16_t voltage_raw = U16(11);
  uint16_t current_raw = U16(13);
  
  if(HobbywingSignatureIdx >= 0) {
    Voltage = calcRangeD(voltage_raw, Signatures[HobbywingSignatureIdx].Voltage, 20);
  }

  
  if(CurrentOffset==0 && current_raw>0){
    CurrentOffset=current_raw;
  }else if(current_raw == 0){
    CurrentOffset=0;
  }

  if((current_raw - CurrentOffset) > 0 && HobbywingSignatureIdx>=0){
    Current = (current_raw - CurrentOffset) * Signatures[HobbywingSignatureIdx].CurrentFactor;
  }else{
    Current = 0.0;
  }

  uint8_t temp1 = U16(15);  
  uint8_t temp2 = U16(17);  
  if(temp1 > 0) Temp1 = calcTemp(temp1);
  if(temp2 > 0) Temp2 = calcTemp(temp2);
  
  char msg[256] = "";
  snprintf(msg, sizeof(msg),
    "%u\t%3.2f\t%3.2f\t%3.2f\t%u\t%3.2f\t%u\t%2.2f\t%u\t%2.2f\t%u\t%u\n",
    frame_id, Thr, OutPWM,    
    Voltage, voltage_raw,
    Current, current_raw,
    Temp1, temp1, Temp2, temp2,
    RPM);

  
  Serial.print(msg);
  bluetoothSerial.print(msg);
}


// Funkcja obliczająca sumę kontrolną CRC16
uint8_t calcCrc(const uint8_t* buf, size_t len) {
  short crc = 0;
  
  for(int i=0;i<len;i++) {
    crc += buf[i]; //0-1FF
    crc += crc >> 8;  //0-100
    crc &= 0x00ff;
    crc += crc >> 8;  //0-0FF
    crc &= 0x00ff;
  }
  return ~crc;
}

void sendToSmartPort(uint16_t type, uint32_t value){
  uint8_t buf[8];

  buf[0]=0x10;
  
  buf[1]=type & 0x00ff;
  buf[2]=(type >> 8) & 0x00ff;

  buf[3]=(value >> 0) & 0x000000ff;
  buf[4]=(value >> 8) & 0x000000ff;
  buf[5]=(value >> 16) & 0x000000ff;
  buf[6]=(value >> 24) & 0x000000ff;

  buf[7]=calcCrc(buf, 7);

  frskySerial.write(buf, 8);
}


void setup() {
  bluetoothSerial.begin("TelemetryBox BT");

  pinMode(escRX_PIN, INPUT);
  pinMode(frskyRX_PIN, INPUT);
  pinMode(frskyTX_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  sleep(5);

  Serial.begin(115200);
  escSerial.begin(19200, SERIAL_8N1, escRX_PIN);//Hobbywing  
  frskySerial.begin(57600, SERIAL_8N1, frskyRX_PIN, frskyTX_PIN, true);//FrSky Smart Port
  
  Serial.printf("=== Welcome in Telemetry Box v%s ===\n", TB_Version);
}

void loop() {
  if(escSerial.available() > 0) {
    uint8_t b = escSerial.read();
    //Serial.printf("%02X ", b);

    if (b == 0x9B) {
      if (escFrameIndex > 0) {        
        if (
          escFrameIndex == 19 || escFrameIndex == 20                    
        ) {
          //printFrame("[HW Frame]", escFrame, escFrameIndex);
          //digitalWrite(LED_PIN, HIGH);
          decodeHwFrame(escFrame, escFrameIndex);          
          //digitalWrite(LED_PIN, LOW);
        }else if(escFrameIndex == 12){
          //Signature frame
          if(HobbywingSignatureIdx<0){
            printFrame("[HW Frame SIG]", escFrame, escFrameIndex);
            HobbywingSignatureIdx = checkSignature(escFrame);
            if(HobbywingSignatureIdx>=0) Serial.printf("Signature found for %s\n", Signatures[HobbywingSignatureIdx].Name);
          }
        }else if(escFrameIndex == 1){
          //Ignore
        }else{
          Serial.printf("[HW Frame] Błąd: ramka o długości %u\n", escFrameIndex);
          printFrame("[HW Frame]", escFrame, escFrameIndex);
        }
      }
      escFrameIndex = 0; // nowa ramka
    }

    if (escFrameIndex < MAX_FRAME_SIZE) {
      escFrame[escFrameIndex++] = b;
    } else {
      escFrameIndex = 0; // reset przy przepełnieniu
      //Serial.println(/*"Błąd: ramka za długa"*/);
    }
  }







  if(frskySerial.available() > 0){
    uint8_t b = frskySerial.read();
    //Serial.printf("%02X ", b);

    if (b == 0x7E) {
      if(frskyFrameIndex > 0) {
        if (frskyFrameIndex == 2) {
          //Serial.printf("Odpytanie o czujnik %02X a czujnik nie odpowiedział\n", frskyFrame[1]);

          /*int i=0;
          bool found=false;
          for(i=0; i<freeIDidx && i<255; i++){
            if(freeIDs[i]==frskyFrame[1]){
              found=true;
              break;
            }
          }

          if(!found && i<255) {
            freeIDs[freeIDidx++]=frskyFrame[1];            
            Serial.printf("Dodawanie wolnego ID FrSky: %02X (%u dodanych ID)\n", frskyFrame[1], freeIDidx);
          }*/
        }else if(frskyFrameIndex == 8){
          Serial.printf("Odpytanie o czujnik %02X - czujnik obecny na linii\n", frskyFrame[1]);
          //Serial.printf("[FrSky Frame] ");
          //for(int i=0; i< frskyFrameIndex; i++) Serial.printf("%02X ", frskyFrame[i]);
          //Serial.println();
          printFrame("[FrSky Frame]", frskyFrame, frskyFrameIndex);
        }
      }
      frskyFrameIndex = 0;
    }

    if (frskyFrameIndex < MAX_FRAME_SIZE) {
      frskyFrame[frskyFrameIndex++] = b;

      //Respond
      if(frskyFrameIndex==2){
        digitalWrite(LED_PIN, LOW);
        //if(frskyFrame[1]==freeIDs[0]) sendToSmartPort(0x0300, 0x82c82c20);//Cells sensor
        if(frskyFrame[1]==0x00){
          sendToSmartPort(0x0900, Voltage * 100.0);//Cells sensor)
          /*else {
            Serial.printf("[FrSky] 0x00 zajęte\n");
            digitalWrite(LED_PIN, HIGH);
          }*/
        } 
        if(frskyFrame[1]==0xa1 && 0 <= Temp1 && Temp1 <= 300.0f){
          sendToSmartPort(0x0400, Temp1);//Temp1
          /*else {
            Serial.printf("[FrSky] 0xa1 zajęte\n");
            digitalWrite(LED_PIN, HIGH);
          }*/
        }
        if(frskyFrame[1]==0x22 && 0 <= Temp2 && Temp2 <= 300.0f){
          sendToSmartPort(0x0410, Temp2);//Temp1)
          /*else {
            Serial.printf("[FrSky] 0x22 zajęte\n");
            digitalWrite(LED_PIN, HIGH);
          }*/
        }
        if(frskyFrame[1]==0x83 && 0 <= RPM && RPM <= 150000){//Daje to 3000obr/min dla 5pp i przełożenia 1:10
          sendToSmartPort(0x0500, RPM);//RPM
          /*else {
            Serial.printf("[FrSky] 0x83 zajęte\n");
            digitalWrite(LED_PIN, HIGH);
          }*/
        }
        if(frskyFrame[1]==0xe4 && 0 <= Current && Current <= 500.0f){
          sendToSmartPort(0x0200, Current);//Current
          /*else {
            Serial.printf("[FrSky] 0xe4 zajęte\n");
            digitalWrite(LED_PIN, HIGH);
          }*/
        }
      }      
    } else {
      frskyFrameIndex = 0;
    }
  }
  
}

