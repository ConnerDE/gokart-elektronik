# DRS System – Gokart (ESP32-S3)

## Überblick
Implementierung eines **DRS-Systems (Drag Reduction System)** für ein Gokart auf Basis eines **ESP32-S3**.  
Der Controller verwaltet Zustand, Sicherheitslogik und Ansteuerung eines hydraulischen Aktuators für einen verstellbaren Heckflügel.

## Funktionen
- Zustandsverwaltung: `Disabled → Armed → Active`
- Manuelle Aktivierung per Lenkradtaster (nur wenn armed)
- Sicherheitslogik mit konfigurierbaren Parametern (Speed, RPM, Gas, Öltemp, Spannung, Bremse)
- Show-Mode per BLE zum Bypassen aller Bedingungen
- BLE-Telemetrie mit DRS-Status

## Hardware
- ESP32-S3
- Hydraulischer Aktuator
- Lenkradtaster via CAN-Bus
- MCP23X17 I/O-Expander

## Projektstruktur

```
a_Unterbau/
├── a_Unterbau.ino          # Globale Definitionen, Objekte, Includes
├── 1_SETUP.ino             # Setup-Routine
├── 2_LOOP.ino              # Hauptloop, Flankenerkennung, Ausgabe
├── b_PIN-Def.ino
├── c_LED-Config.ino
├── d_TMC2209-Config.ino
├── e_OLED-Config.ino
├── f_MCP23017-Mapping.ino
├── g_RPM-Counter.ino
├── h_CAN-Receiver.ino
├── i_GAS-Servo.ino
├── j_SAFETY-STARTSTOP.ino
├── k_GEARBOX.ino
├── l_EXHAUST-SERVO.ino
├── m_LIGHTS.ino
├── n_BLE.ino
├── o_DISPLAY.ino
├── p_DRS.ino               # Zustandsmaschine, Getter
└── q_DRS_Config.ino        # Parameter, Bedingungslogik, Show-Mode
```
- Muss noch geupdated werden.

## BLE Commands

### Kalibration
| Command | Funktion |
|---|---|
| `CAL:GAS_MIN` | Gaspedalposition als Minimum speichern |
| `CAL:GAS_MAX` | Gaspedalposition als Maximum speichern |
| `SET:SRV_GAS_MIN:<wert>` | Servo Gas Minimalwinkel setzen |
| `SET:SRV_GAS_MAX:<wert>` | Servo Gas Maximalwinkel setzen |
| `SET:SRV_EXH_MIN:<wert>` | Servo Auspuff Minimalwinkel setzen |
| `SET:SRV_EXH_MAX:<wert>` | Servo Auspuff Maximalwinkel setzen |

### Reset
| Command | Funktion |
|---|---|
| `RESET_HOURS` | Betriebsstunden auf 0 zurücksetzen |
| `RESET_CHAIN` | Kettenschaltzähler auf 0 zurücksetzen |

### Sonstiges
| Command | Funktion |
|---|---|
| `SPD:<wert>` | Geschwindigkeit manuell setzen (float) |
| `DRS:SHOW_ON` | DRS Show-Mode aktivieren (bypasses alle Bedingungen) |
| `DRS:SHOW_OFF` | DRS Show-Mode deaktivieren |
## License
![License: Proprietary](https://shields.io)
This project is proprietary. All rights reserved.

