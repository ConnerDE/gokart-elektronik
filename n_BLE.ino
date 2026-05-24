
/* ==================== BLE ==================== */
class BLECallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override {
    String value = pChar->getValue();
    if (value.length() > 0) {
      String s = String(value.c_str());

      // Calibration Commands
      if (s == "CAL:GAS_MIN") {
        gasPedal.setPedalMinFromCurrent();
        calGasMin = gasPedal.getRawPos();
      } else if (s == "CAL:GAS_MAX") {
        gasPedal.setPedalMaxFromCurrent();
        calGasMax = gasPedal.getRawPos();
      } else if (s == "CAL:STEER_LEFT") {
        steering.setLeftLimit();
      } else if (s == "CAL:STEER_CENTER") {
        steering.setCenter();
      } else if (s == "CAL:STEER_RIGHT") {
        steering.setRightLimit();
      } else if (s.startsWith("SET:SRV_GAS_MIN:")) {
        calSrvGasMin = constrain(s.substring(16).toInt(), 0, 180 - GAS_SERVO_TRAVEL_DEG);
        calSrvGasMax = constrain(calSrvGasMax, calSrvGasMin, calSrvGasMin + GAS_SERVO_TRAVEL_DEG);
        prefs.putInt(PREF_SRV_GAS_MIN, calSrvGasMin);
        prefs.putInt(PREF_SRV_GAS_MAX, calSrvGasMax);
      } else if (s.startsWith("SET:SRV_GAS_MAX:")) {
        calSrvGasMax = constrain(s.substring(16).toInt(), calSrvGasMin, calSrvGasMin + GAS_SERVO_TRAVEL_DEG);
        prefs.putInt(PREF_SRV_GAS_MAX, calSrvGasMax);
      } else if (s.startsWith("SET:SRV_EXH_MIN:")) {
        calSrvExhMin = constrain(s.substring(16).toInt(), 0, 180);
        prefs.putInt(PREF_SRV_EXH_MIN, calSrvExhMin);
      } else if (s.startsWith("SET:SRV_EXH_MAX:")) {
        calSrvExhMax = constrain(s.substring(16).toInt(), 0, 180);
        prefs.putInt(PREF_SRV_EXH_MAX, calSrvExhMax);
      }

      // Reset Commands
      else if (s == "RESET_HOURS") {
        engineHours = 0;
        prefs.putFloat(PREF_HOURS, 0.0);
      } else if (s == "RESET_CHAIN") {
        chainShifts = 0;
        prefs.putUInt(PREF_CHAIN, 0);
      } else if (s.startsWith("SPD:")) {
        currentSpeed = constrain(s.substring(4).toFloat(), 0.0f, 255.0f);
      }
      // DRS Show Mode
      else if (s == "DRS:SHOW_ON") {
        setDRSShowMode(true);
      } else if (s == "DRS:SHOW_OFF") {
        setDRSShowMode(false);
      }
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override { deviceConnected = true; }
  void onDisconnect(BLEServer*) override { deviceConnected = false; }
};

void sendTelemetry(SafetyModule& safety, CanReceiver& can, GasPedal& gas, int8_t gear, bool launch) {
  static unsigned long last = 0;
  if (!deviceConnected || millis() - last < 500) return;
  last = millis();

  JsonDocument doc;
  doc["gear"] = gear;
  if (can.chassisDataValid) {
    doc["vRef"] = can.vRefKmh;
    doc["vDrv"] = can.vDriveKmh;
    doc["asr"] = can.shouldCutForSlip();
    doc["slip"] = can.slipLampActive();
    doc["slipPct"] = can.currentSlip;
    doc["slipLimit"] = can.slipLimitPercent();
    doc["asrErr"] = can.asrSensorError;
    doc["asrOff"] = can.asrOff;
    doc["drift"] = can.driftMode;
    doc["vVL"] = can.speedVLKmh;
    doc["vVR"] = can.speedVRKmh;
    doc["vHA"] = can.speedHAKmh;
  }
  doc["rpm"] = safety.currentRPM;
  doc["bat"] = safety.batteryVoltage;
  doc["oil"] = safety.oilTemp;
  doc["mode"] = can.driveMode;
  doc["gas"] = gas.getGasPercent();
  doc["launch"] = launch;
  doc["starter"] = safety.isStarterActive();
  doc["light"] = can.lightMode;
  doc["rgb"] = can.rgbMode;
  doc["env"] = safety.outsideTemp;
  doc["press"] = safety.airPressure;
  doc["hours"] = engineHours;
  doc["shifts"] = totalShifts;

  // Stromsensor Daten (NEU!)
  doc["curr_mA"] = currentSensor.getCurrentMA();
  doc["power_W"] = currentSensor.PowerWatt();
  doc["curr_raw"] = ads1115.readCurrentSensor();
  doc["egt"] = egtSensor.getTemperatureC();
  doc["egtOK"] = egtSensor.isValid();
  doc["steerRaw"] = steering.readRaw();

  // OBD Diagnostics (Bitmask)
  uint32_t diag = 0;
  if (safety.brakePressed) diag |= (1 << 0);
  if (safety.neutralActive) diag |= (1 << 1);
  if (safety.tiltDetected) diag |= (1 << 2);
  if (safety.oilPresent) diag |= (1 << 3);
  if (safety.ignitionOn) diag |= (1 << 4);
  if (can.dataValid) diag |= (1 << 5);
  // I2C Status (Bits 8-15)
  diag |= (i2cStatus << 8);

  doc["diag"] = diag;

  if (engineHours > SERVICE_HOURS_LIMIT) doc["warn"] = "SERVICE";
  else if (chainShifts > CHAIN_SHIFTS_LIMIT) doc["warn"] = "CHAIN";
  else doc["warn"] = "OK";

  String exhStr = "Closed";
  if (can.exhaustAngle == 45) exhStr = "Partial";
  if (can.exhaustAngle == 90) exhStr = "Open";
  doc["exh"] = exhStr;
  doc["exhAng"] = can.exhaustAngle;
  doc["drs"]      = (uint8_t)getDRSState();  // 0=Disabled, 1=Armed, 2=Active
  doc["drsShow"]  = drsShowMode;

  String json;
  serializeJson(doc, json);
  pCharacteristic->setValue(json.c_str());
  pCharacteristic->notify();
}
