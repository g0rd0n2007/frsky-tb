# FrSky Telemetry Box
Read data from HobbyWing ESCs and send it via FrSky Smart Port to Your TX.

## HobbyWing ESC
Communication details:
- 19 200 bps
- Big endian coding
- bit duration: 52 us
- two types of frames sent: ID, data
- Normal logic

Timechart:
![Hobbywing](https://github.com/g0rd0n2007/frsky-tb/blob/main/Zrzut%20ekranu%20z%202023-10-01%2012-06-21.png)

Frame with ID:
|                | 1    | 2    | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   | 11   | 12   | 13   |
| -------------- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| V4LV/25/60/80A | 0x9b | 0x9b | 0x03 | 0xe8 | 0x01 | 0x08 | 0x5b | 0x00 | 0x01 | 0x00 | 0x21 | 0x21 | 0xb9 |
| V4HV200A OPTO  | 0x9b | 0x9b | 0x03 | 0xe8 | 0x01 | 0x02 | 0x0d | 0x0a | 0x3d | 0x05 | 0x1e | 0x21 | 0xb9 |
| V5HV130A OPTO  | 0x9b | 0x9b | 0x03 | 0xe8 | 0x01 | 0x0b | 0x41 | 0x21 | 0x44 | 0xb9 | 0x21 | 0x21 | 0xb9 |
| HW HV 200A     | 0x9b | 0x9b | 0x02 | 0xd0 | 0x01 | 0x0b | 0x41 | 0x21 | 0x7e | 0x62 | 0x21 | 0x21 | 0xb9 |

Frame with data:
| Byte        | 1    | 2-4          | 5-6    | 7-8     | 9-11         | 12-13   | 14-15   | 16-17    | 18-19 |
| ----------- |  --- | ------------ | ------ | ------- | ------------ | ------- | ------- | -------- | ----- |
| Description | Head | Frame number | Rx Thr | Out PWM | RPM raw data | Voltage | Current | Temp FET | Temp  |
| Value       | 0x9b |              | 0-1023 | 0-1023  |              |         |         |          |       |


Rx Thr, Out PWM: 
- 0-1023
  
RPM:
-	Motor RPM = RPM_raw_data / No_of_pole_pairs (set it in TX)


## FrSky Smart Port

Communication details:
- 57 600 bps
- Little endian coding
- bit duration: 17 us
- Inverted logic

Timechart:
![Smart Port](https://github.com/g0rd0n2007/frsky-tb/blob/main/Zrzut%20ekranu%20z%202023-10-01%2012-18-04.png)

* Connect MCU Tx Pin through 120 Ohm resistor.

Receiver pools sensor with:
| Byte | 1    | 2         |
| ---- | ---- | --------- |
|      | 0x7e | sensor_id |

If sensor is present - it answers with:
| Byte | 1    | 2-3         | 4-7   | 8   |
| ---- | ---- | ----------- | ----- | --- |
|      | Head | Sensor type | Value | CRC |
|      | 0x10 |             |       |     |

Pooled sensor IDs:
- 6A CB AC 0D 8E 2F D0 71 F2 53 34 95 16 B7 98 39 BA 1B 00 A1 22 83 E4 45 C6 67 48 E9

Known sensor types:
- RPM: 0x0500
- A3 (Voltage): 0x0900
- A4: 0x0910
- Current: 0x0200
- T1: 0x0400
- T2: 0x0410
- FLVV Cell sensor: 0x0300

CRC Calculation code:
```C
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
```

Bytes sequence imitating FLVV Cell sensor for 2S LiPo:
- 0x10, 0x00, 0x03, 0x20, 0x2c, 0xc8, 0x82
- First is the head
- Sensor type is 0x0300,
- 20 means:
  - Total numbers of cells is 2
  - Currently sent ID is 0
    - For 4S battery there will be frames with ID=0 (Cell0=C0 and Cell1=C1) and ID=2 (Cell0 refers to C2 and Cell1 refers to C3)    
- Two cells voltages are send using 24 + 24 bits.
  - Cell0: 0x82c -> when read as float it refers to 4.2V
  - Cell1: 0x82c
- Transmitter will sum up all cells and show summed voltage  
