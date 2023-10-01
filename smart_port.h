//Smart Port
//SP_Send(SP, 0x0300, 0x82c82c20);


#define FRSKY_SP_RPM  0x0500
#define FRSKY_SP_A3   0x0900
#define FRSKY_SP_A4   0x0910
#define FRSKY_SP_CURR 0x0200
#define FRSKY_SP_T1   0x0400
#define FRSKY_SP_T2   0x0410

uint8_t CalculateCRC(uint8_t bytes[], int len){
  short crc = 0;
  
  for(int i=0;i<len;i++) {
    crc += bytes[i]; //0-1FF
    crc += crc >> 8;  //0-100
    crc &= 0x00ff;
    crc += crc >> 8;  //0-0FF
    crc &= 0x00ff;
  }
  return ~crc;
}

void SP_SendPacket(SerialConfiguration sc, uint8_t bytes[]){
  uint8_t crc = CalculateCRC(bytes, 7);
  
  for(int i=0;i<7;i++){
    SerialSendByte(sc, bytes[i]);
    //Serial.print(bytes[i], HEX);
    //Serial.print(" ");
  }  
  
  SerialSendByte(sc, crc);
  //Serial.print(crc, HEX);
  //Serial.println();
}

void SP_Send(SerialConfiguration sc, uint16_t typ, uint32_t v){
  uint8_t bytes[7];

  bytes[0]=0x10;
  
  bytes[1]=typ & 0x00ff;
  bytes[2]=(typ >> 8) & 0x00ff;

  bytes[3]=(v >> 0) & 0x000000ff;
  bytes[4]=(v >> 8) & 0x000000ff;
  bytes[5]=(v >> 16) & 0x000000ff;
  bytes[6]=(v >> 24) & 0x000000ff;

  SP_SendPacket(sc, bytes);
}
