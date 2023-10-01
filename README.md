# FrSky Telemetry Box
Read data from HobbyWing ESCs and send it via FrSky Smart Port to Your TX.

## HobbyWing ESC
Communication details:
- 19 200 bps
- Big endian coding
- bit duration: 52 us
- two types of frames sent: ID, data


Frame with ID:
|                | 1    | 2    | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   | 11   | 12   |
| -------------- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| V4LV/25/60/80A | 0x9b | 0x9b | 0x03 | 0xe8 | 0x01 | 0x08 | 0x5b | 0x00 | 0x01 | 0x00 | 0x21 | 0x21 |
| V4HV200A OPTO  | 0x9b | 0x9b | 0x03 | 0xe8 | 0x01 | 0x02 | 0x0d | 0x0a | 0x3d | 0x05 | 0x1e | 0x21 |
| V5HV130A OPTO  | 0x9b | 0x9b | 0x03 | 0xe8 | 0x01 | 0x0b | 0x41 | 0x21 | 0x44 | 0xb9 | 0x21 | 0x21 |

Frame with data:
| Byte        | 1    | 2-4          | 5-6    | 7-8     | 9-11         | 12-13   | 14-15   | 16-17    | 18-19 |
| ----------- |  --- | ------------ | ------ | ------- | ------------ | ------- | ------- | -------- | ----- |
| Description | Head | Frame number | Rx Thr | Out PWM | RPM raw data | Voltage | Current | Temp FET | Temp  |
| Value       | 0x9b |              | 0-1023 | 0-1023  |              |         |         |          |       |


Rx Thr, Out PWM: 
- 0-1023
  
RPM:
-	Motor RPM = RPM_raw_data / No_of_pole_pairs

Voltage:
- V = Voltage / 114
  
Temp:
- *C = Temp / 100


