/* ==================== GAS PEDAL & SERVO ==================== */
class GasPedal {
  int32_t encoderPos = 0;
  uint8_t lastEncState = 0;
  uint8_t currentGasPercent = 0;
  uint8_t asrCutPercent = 0;

  bool calibrating = false;
  int32_t calLearnMin = GAS_MAX_STEPS_RAW;
  int32_t calLearnMax = 0;

  void writeServoAngle(int targetAngle) {
    uint32_t pulseWidth_us = map(targetAngle, 0, 180, 1000, 2000);
    uint32_t duty = (uint32_t)((pulseWidth_us * (uint64_t)LEDC_SERVO_DUTY_MAX) / 20000ULL);
    ledc_set_duty(LEDC_SERVO_MODE, LEDC_SERVO_GAS_CH, duty);
    ledc_update_duty(LEDC_SERVO_MODE, LEDC_SERVO_GAS_CH);
  }

public:
  void begin() {
    ledc_channel_config_t gas_conf = {
      .gpio_num = PIN_GAS_SERVO,
      .speed_mode = LEDC_SERVO_MODE,
      .channel = LEDC_SERVO_GAS_CH,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_SERVO_TIMER,
      .duty = 0,
      .hpoint = 0,
      .flags = {}
    };
    ledc_channel_config(&gas_conf);

    calGasMin = prefs.getInt(PREF_GAS_MIN, 0);
    calGasMax = prefs.getInt(PREF_GAS_MAX, 60);
    calSrvGasMin = prefs.getInt(PREF_SRV_GAS_MIN, 0);
    calSrvGasMax = prefs.getInt(PREF_SRV_GAS_MAX, 180);

    calibrating = true;
    calLearnMin = GAS_MAX_STEPS_RAW;
    calLearnMax = 0;
  }

  void cutThrottle() {
    writeServoAngle(calSrvGasMin);
  }

  void updateCalibration() {
    if (!calibrating) return;

    bool a = mcp1.digitalRead(MCP1_PEDAL_ENC_A);
    bool b = mcp1.digitalRead(MCP1_PEDAL_ENC_B);
    uint8_t state = (a << 1) | b;

    if (state != lastEncState) {
      if ((lastEncState == 0b00 && state == 0b01) || (lastEncState == 0b01 && state == 0b11) ||
          (lastEncState == 0b11 && state == 0b10) || (lastEncState == 0b10 && state == 0b00)) {
        encoderPos++;
      } else {
        encoderPos--;
      }
      lastEncState = state;
      encoderPos = constrain(encoderPos, 0, GAS_MAX_STEPS_RAW);
    }

    if (encoderPos < calLearnMin) calLearnMin = encoderPos;
    if (encoderPos > calLearnMax) calLearnMax = encoderPos;

    if ((calLearnMax - calLearnMin) > 20 && encoderPos <= calLearnMin + 2) {
      calGasMin = calLearnMin;
      calGasMax = calLearnMax;
      prefs.putInt(PREF_GAS_MIN, calGasMin);
      prefs.putInt(PREF_GAS_MAX, calGasMax);
      calibrating = false;
    }
  }

  bool isCalibrating() { return calibrating; }

  void update(uint8_t driveMode, bool launchActive, uint16_t currentRPM, bool neutralActive,
              bool asrActive, bool driftMode, uint8_t vRefKmh, int8_t steerPercent) {
    (void)neutralActive;

    bool a = mcp1.digitalRead(MCP1_PEDAL_ENC_A);
    bool b = mcp1.digitalRead(MCP1_PEDAL_ENC_B);
    uint8_t state = (a << 1) | b;

    if (state != lastEncState) {
      if ((lastEncState == 0b00 && state == 0b01) || (lastEncState == 0b01 && state == 0b11) ||
          (lastEncState == 0b11 && state == 0b10) || (lastEncState == 0b10 && state == 0b00)) {
        encoderPos++;
      } else {
        encoderPos--;
      }
      lastEncState = state;
      encoderPos = constrain(encoderPos, 0, GAS_MAX_STEPS_RAW);
    }

    int32_t constrainedPos = constrain(encoderPos, calGasMin, calGasMax);
    currentGasPercent = map(constrainedPos, calGasMin, calGasMax, 0, 100);

    float inputNorm = (float)(constrainedPos - calGasMin) / (float)(calGasMax - calGasMin);
    float outputNorm = inputNorm;

    switch (driveMode) {
      case 1: outputNorm = pow(inputNorm, 1.5); break;
      case 2: outputNorm = inputNorm * inputNorm; break;
      case 3: outputNorm = constrain(inputNorm * 1.2, 0.0, 1.0); break;
    }

    if (driftMode) {
      outputNorm = constrain(inputNorm * 1.15f, 0.0f, 1.0f);
    }

    int targetAngle = (int)(outputNorm * (calSrvGasMax - calSrvGasMin) + calSrvGasMin);

    if (launchActive && currentRPM > 2100) {
      targetAngle = calSrvGasMin;
    }

    if (asrActive) {
      if (asrCutPercent < 95) asrCutPercent += driftMode ? 1 : 3;
    } else if (asrCutPercent > 0) {
      asrCutPercent = (asrCutPercent > 8) ? asrCutPercent - 8 : 0;
    }

    if (asrCutPercent > 0) {
      int span = calSrvGasMax - calSrvGasMin;
      int cut = (span * asrCutPercent) / 100;
      targetAngle = max(calSrvGasMin, targetAngle - cut);
    }

    if ((float)vRefKmh > ROLLOVER_SPEED_KMH && abs(steerPercent) > ROLLOVER_STEER_PCT) {
      int steerMag = abs(steerPercent);
      int maxGasPct = map(steerMag, ROLLOVER_STEER_PCT, 100, 70, 25);
      int maxAngle = calSrvGasMin + (calSrvGasMax - calSrvGasMin) * maxGasPct / 100;
      if (targetAngle > maxAngle) targetAngle = maxAngle;
    }

    writeServoAngle(targetAngle);
  }

  uint8_t getGasPercent() { return currentGasPercent; }
  int32_t getRawPos() { return encoderPos; }
};
