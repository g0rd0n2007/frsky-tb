//Serial

#define SerialBufSize 20

typedef struct{
  long BitDuration;//us
  long Speed;//bps
  bool Inverted;
  int RxPin, TxPin;
  byte B[SerialBufSize];
}SerialConfiguration;

SerialConfiguration SerialConfigure(long speed, bool inv, int rx_pin, int tx_pin){
  SerialConfiguration o;

  o.BitDuration=1000000/speed-1;//Lepiej z minus jeden
  o.Speed=speed;
  o.Inverted=inv;
  o.RxPin=rx_pin;
  o.TxPin=tx_pin;

  //Prepare buffer
  for(int i=0;i<SerialBufSize;i++)o.B[i]=0;
  
  if(rx_pin>=0) {
    pinMode(rx_pin, INPUT_PULLUP);
    //digitalWrite(rx_pin, HIGH);
  }
  if(tx_pin>=0) {
    pinMode(tx_pin, OUTPUT);
    digitalWrite(tx_pin, LOW);
  }

  return o;
}

unsigned long MeasurePinState(SerialConfiguration sc, bool state){
  unsigned long start, finish;

  start=micros();
  while(digitalRead(sc.RxPin)==state);
  finish=micros();

  return finish-start;
}

void SerialSynchronize(SerialConfiguration sc){
  unsigned long m0, m1;

  //Waiting for line free
  if(sc.Inverted){
    while(1){
      m1=MeasurePinState(sc, 1);
      m0=MeasurePinState(sc, 0);

      if(m0>=(9.5*sc.BitDuration)) break;
    }
  }else{
    while(1){
      m0=MeasurePinState(sc, 0);
      m1=MeasurePinState(sc, 1);

      if(m1>=(9.5*sc.BitDuration)) break;
    }  
  }
}

byte SerialReceiveByte(SerialConfiguration *sc){
  byte o;


  if(sc->Inverted){
    o=255;

    
    //Stop and wait if line is LOW, we assume at this point should be HIGH
    while(digitalRead(sc->RxPin)==LOW);
    //Now it is HIGH for sure



    //Start Bit skip
    delayMicroseconds(sc->BitDuration);
  

    //Waiting for LSB middle
    delayMicroseconds(sc->BitDuration/2);
    o &= ~(digitalRead(sc->RxPin) << 0);

    delayMicroseconds(sc->BitDuration);
    o &= ~(digitalRead(sc->RxPin) << 1);

    delayMicroseconds(sc->BitDuration);
    o &= ~(digitalRead(sc->RxPin) << 2);

    delayMicroseconds(sc->BitDuration);
    o &= ~(digitalRead(sc->RxPin) << 3);

    delayMicroseconds(sc->BitDuration);
    o &= ~(digitalRead(sc->RxPin) << 4);

    delayMicroseconds(sc->BitDuration);
    o &= ~(digitalRead(sc->RxPin) << 5);

    delayMicroseconds(sc->BitDuration);
    o &= ~(digitalRead(sc->RxPin) << 6);

    //MSB middle
    delayMicroseconds(sc->BitDuration);
    o &= ~(digitalRead(sc->RxPin) << 7);


    //Save data to buffer
    //Serial.print(o);
    for(int i=0;i<(SerialBufSize-1);i++) sc->B[i]=sc->B[i+1];
    sc->B[SerialBufSize-1]=o;
    //Serial.println(sc->B[SerialBufSize-2]);

    //If MSB is 1 then use it to synchronize communication, because we know that Stop bit will be 0
    if(digitalRead(sc->RxPin)==1){
      MeasurePinState(*sc, 1);
    }else{
      delayMicroseconds(sc->BitDuration/2);
    }
  }else{
    o=0;

    
    //Stop and wait if line is HIGH, we assume at this point should be LOW
    while(digitalRead(sc->RxPin)==HIGH);
    //Now it is LOW for sure



    //Start Bit skip
    delayMicroseconds(sc->BitDuration);
  

    //Waiting for LSB middle
    delayMicroseconds(sc->BitDuration/2);
    o |= digitalRead(sc->RxPin) << 0;

    delayMicroseconds(sc->BitDuration);
    o |= digitalRead(sc->RxPin) << 1;

    delayMicroseconds(sc->BitDuration);
    o |= digitalRead(sc->RxPin) << 2;

    delayMicroseconds(sc->BitDuration);
    o |= digitalRead(sc->RxPin) << 3;

    delayMicroseconds(sc->BitDuration);
    o |= digitalRead(sc->RxPin) << 4;

    delayMicroseconds(sc->BitDuration);
    o |= digitalRead(sc->RxPin) << 5;

    delayMicroseconds(sc->BitDuration);
    o |= digitalRead(sc->RxPin) << 6;

    //MSB middle
    delayMicroseconds(sc->BitDuration);
    o |= digitalRead(sc->RxPin) << 7;


    //Save data to buffer
    //Serial.print(o);
    for(int i=0;i<(SerialBufSize-1);i++) sc->B[i]=sc->B[i+1];
    sc->B[SerialBufSize-1]=o;
    //Serial.println(sc->B[SerialBufSize-2]);

    //If MSB is 0 then use it to synchronize communication, because we know that Stop bit will be 1
    if(digitalRead(sc->RxPin)==0){
      MeasurePinState(*sc, 0);
    }else{
      delayMicroseconds(sc->BitDuration/2);
    }
  }
  
  //Stop Bit
  delayMicroseconds(sc->BitDuration);

  return o;
}

void SerialSendByte(SerialConfiguration sc, uint8_t b){
  if(sc.Inverted){
    //Serial.print("x ");
    //Start Bit
    digitalWrite(sc.TxPin, HIGH);
    delayMicroseconds(sc.BitDuration);

    for(int i=0;i<8;i++){
      digitalWrite(sc.TxPin, (b & (1<<i)) ? LOW : HIGH);
      delayMicroseconds(sc.BitDuration);
    }

    //Stop Bit
    digitalWrite(sc.TxPin, LOW);
    delayMicroseconds(sc.BitDuration);
  }else{
    //Start Bit
    digitalWrite(sc.TxPin, LOW);
    delayMicroseconds(sc.BitDuration);

    for(int i=0;i<8;i++){
      digitalWrite(sc.TxPin, (b & (1<<i)) ? HIGH : LOW);
      delayMicroseconds(sc.BitDuration);
    }

    //Stop Bit
    digitalWrite(sc.TxPin, HIGH);
    delayMicroseconds(sc.BitDuration);
  }
}



void PrintBuffer(SerialConfiguration sc, int start, int finish){
  for(int i=start;i<finish;i++) {
    Serial.print(sc.B[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
