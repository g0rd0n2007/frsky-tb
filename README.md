# FrSky Telemetry Box

Read data from HobbyWing ESCs and send it via FrSky Smart Port to Your TX.

## ESP32 Board

Telemetry box uses ESP32 board. Details:

- VIN: 3.3-12V gives very wide voltage range

- GPIO32: Connected to Hobbywing ESC for telemetry data.

- GPIO33: It is SmartPort RX signal. It reads the line. Connect it to model's receiver SmartPort

- GPIO14: It is SmartPort TX signal. It is used to transmit data to receiver's SmartPort. Use 120-220ohm resistor. After resistor - join it with GPIO33 and this needs to be bi-directional communication.
  
  
  
  Pinout:
  
  ![pinout.jpg](/images/pinout.jpg)
  
  

## HobbyWing ESC

Communication details:

- 19 200 bps
- Parity bit: No
- Stop bit: 1
- Big endian coding
- Bit duration: 52 us
- Two types of frames sent
  - Hobbywing signature frame (13 bytes)
  - Data frame (19-20bytes)
- Logic: normal
  
  

Timechart:


![1.png](/home/dariusz-gogacz/Dokumenty/frsky-tb/images/1.png)



### Data frames

Length equal to 19-20 bytes. 

- 0: Start byte 0x9B

- 1-3: Frame number

- 4-5: 0-1000 means 0-100% of RX Throttle. Divide this value by 10.0 to get percent value.

- 6-7: 0-1000 means 0-100% of Motor PWM output. Divide this value by 10.0 to get percent value.

- 8-10: RPM raw value. Motor RPM = RPM_raw_data / No_of_pole_pairs (set it in TX, usualy equal to 5)

- 11-12: Voltage (raw)

- 13-14: Current (raw)

- 15-16: Temperature 1

- 17-18: Temperature 2

- 19: Optional. Present when motor is off. Equal to 0xB9



Examples:

|      | Start | Frame number | RX Throttle | Output PWM | RPM    | Voltage | Current | Temp1 | Temp2 | End  |
| ---- | ----- | ------------ | ----------- | ---------- | ------ | ------- | ------- | ----- | ----- | ---- |
| Byte | 0     | 1-3          | 4-5         | 6-7        | 8-10   | 11-12   | 13-14   | 15-16 | 17-18 | 19   |
|      | 0x9B  | 0007DC       | 012C        | 0000       | 000000 | 0A49    | 0000    | 0C80  | 0C98  | 0xB9 |
|      | 0x9B  | 0007DD       | 012C        | 0000       | 000000 | 0A4A    | 0000    | 0C80  | 0C97  | 0xB9 |
|      | 0x9B  | 0007DE       | 012C        | 0000       | 000000 | 0A49    | 0000    | 0C80  | 0C98  | 0xB9 |
|      | 0x9B  | 0007DF       | 012C        | 0000       | 000000 | 0A4A    | 0000    | 0C80  | 0C97  | 0xB9 |
|      | 0x9B  | 0007E0       | 012C        | 0000       | 000000 | 0A48    | 0000    | 0C7E  | 0C98  | 0xB9 |
|      | 0x9B  | 0007E1       | 012C        | 0000       | 000000 | 0A4C    | 0000    | 0C82  | 0C9A  | 0xB9 |
|      | 0x9B  | 0007E2       | 012C        | 0000       | 000000 | 0A48    | 0000    | 0C80  | 0C96  | 0xB9 |
|      | 0x9B  | 0007E3       | 012C        | 0000       | 000000 | 0A48    | 0000    | 0C80  | 0C95  | 0xB9 |
|      | 0x9B  | 0007E4       | 012C        | 0000       | 000000 | 0A4B    | 0000    | 0C80  | 0C98  | 0xB9 |
|      | 0x9B  | 0007E5       | 012C        | 0000       | 000000 | 0A48    | 0000    | 0C80  | 0C9A  | 0xB9 |
|      | 0x9B  | 0007E6       | 012C        | 0000       | 000000 | 0A49    | 0000    | 0C80  | 0C98  | 0xB9 |
|      | 0x9B  | 0007E7       | 012C        | 0000       | 000000 | 0A48    | 0000    | 0C80  | 0C98  | 0xB9 |



### Signature frames

Length equal to 13 bytes. But it sends two bytes 0x9B and my software sees it as 12 byte frame. I use 0x9B to trigger analysis and therefore I analyse 12 bytes.



|                | Start | Start |      |      |      |      |      |      |      |      |      |      | End  |
| -------------- | ----- | ----- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| Byte           | -1    | 0     | 1    | 2    | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   | 11   |
| V4LV/25/60/80A | 0x9B  | 0x9B  | 0x03 | 0xE8 | 0x01 | 0x08 | 0x5B | 0x00 | 0x01 | 0x00 | 0x21 | 0x21 | 0xB9 |
| V4HV200A OPTO  | 0x9B  | 0x9B  | 0x03 | 0xE8 | 0x01 | 0x02 | 0x0D | 0x0A | 0x3D | 0x05 | 0x1E | 0x21 | 0xB9 |
| V5HV130A OPTO  | 0x9B  | 0x9B  | 0x03 | 0xE8 | 0x01 | 0x0B | 0x41 | 0x21 | 0x44 | 0xB9 | 0x21 | 0x21 | 0xB9 |
| HW HV 200A     | 0x9B  | 0x9B  | 0x02 | 0xD0 | 0x01 | 0x0B | 0x41 | 0x21 | 0x7E | 0x62 | 0x21 | 0x21 | 0xB9 |
| HW 120A        | 0x9B  | 0x9B  | 0x03 | 0xE8 | 0x01 | 0x08 | 0x5B | 0x21 | 0x71 | 0x6E | 0x21 | 0x21 | 0xB9 |



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
