/* ==================== CURRENT SENSOR (ACS770) ==================== */
class CurrentSensor {
  float currentMA = 0;
  float powerWatt = 0;
  uint16_t batteryVoltageADC = 0;

  static constexpr float OFFSET = 2.5f;
  static constexpr float SENSITIVITY = 166.0f;

public:
  void begin() {
    Serial.println("Initializing Current Sensor...");
  }

  void update(uint16_t batteryVoltage_mV) {
    if (!ads1115.initialized) return;

    int16_t currentRaw = ads1115.readCurrentSensor();
    float voltage = ads1115.rawToVolts(currentRaw);
    float offsetVoltage = voltage - OFFSET;
    currentMA = (offsetVoltage * 1000.0f) / SENSITIVITY;
    powerWatt = (float)batteryVoltage_mV / 1000.0f * abs(currentMA) / 1000.0f;
    batteryVoltageADC = batteryVoltage_mV;
  }

  float getSensorVoltageV() const {
    return ads1115.rawToVolts(ads1115.readCurrentSensor());
  }

  float getCurrentMA() const { return currentMA; }
  float getCurrentA() const { return currentMA / 1000.0f; }
  float PowerWatt() const { return powerWatt; }
  uint16_t getBatteryVoltage() const { return batteryVoltageADC; }

  static float calculateEnergyWh(float powerW, unsigned long timeMS) {
    return (powerW * timeMS) / (1000.0f * 3600.0f);
  }
};
