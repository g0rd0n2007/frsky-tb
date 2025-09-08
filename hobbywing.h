struct Range{
  int range;
  float factor;
};

struct Signature {
  byte Sig[12];
  const char* Name;
  //float VoltageDivisor;
  float CurrentFactor;
  Range Voltage[20];
};

Signature Signatures[] = {                                                                   //3.10, 2.58
  {{0x9b,	0x02,	0xd0,	0x01,	0x0b,	0x41,	0x21,	0x7e,	0x62,	0x21,	0x21,	0xb9}, "Hobbywing 200A", 2.45f, {
    {0, 58.0}
  }},//                                                                                        3.10, 2.58
  {{0x9b,	0x03,	0xe8,	0x01,	0x08,	0x5b,	0x21,	0x71,	0x6e,	0x21,	0x21,	0xb9}, "Hobbywing 120A", 2.45f, {
    {2840, 113.60f}, //25.00V real battery
    {2585, 113.70f}, //22.86V real
    /*{2531, 105.45f}, //24V
    {2410, 104.78f}, //23V
    {2288, 104.00f}, //22V
    {2165, 103.09f}, //21V
    {2032, 101.60f}, //20V
    {1912, 100.63f}, //19V 
    {1782, 99.00f }, //18V
    {1651, 97.11f }, //17V
    {1518, 94.87f }, //16V
    {1376, 91.73f }, //15V
    {1224, 87.45f }, //14V
    {1075, 82.69f }, //13V
    {934,  77.83f }, //12V*/
  }}
};

const int signaturesCount = sizeof(Signatures) / sizeof(Signatures[0]);

const int checkSignature(byte* buf) {
  for (int i = 0; i < signaturesCount; i++) {
    bool match = true;
    for (int j = 0; j < 12; j++) {
      if (buf[j] != Signatures[i].Sig[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return i;
    }
  }

  return -1; // brak dopasowania
}

float calcRangeD(int x, Range t[], int size) {
  int lastActiveIndex = 0;
  for(int i = 0; i < size; i++) {
    if(t[i].factor != 0) lastActiveIndex = i;
  }

  if(x >= t[0].range) return x / t[0].factor;

  // iteracja od góry do ostatniego aktywnego punktu
  for(int i = 0; i < lastActiveIndex; i++) {
    if(x >= t[i+1].range) {
      float factor = t[i].factor + ((float)(x - t[i].range) * (t[i+1].factor - t[i].factor) / (t[i+1].range - t[i].range));
      return x / factor;
    }
  }

  return x / t[lastActiveIndex].factor;
}