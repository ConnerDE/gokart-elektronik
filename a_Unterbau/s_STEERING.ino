
/* ==================== LENKWINKEL (ANALOG) ==================== */
class SteeringInput {
  int centerRaw = 2048;
  static constexpr int DEADZONE = 80;

public:
  void begin() {
    analogReadResolution(12);
    pinMode(PIN_STEER_ADC, INPUT);
    delay(10);
    centerRaw = analogRead(PIN_STEER_ADC);
  }

  int8_t getSteerPercent() {
    int raw = analogRead(PIN_STEER_ADC);
    int delta = raw - centerRaw;
    if (abs(delta) < DEADZONE) return 0;
    int pct = map(abs(delta), DEADZONE, 2048, 0, 100);
    pct = constrain(pct, 0, 100);
    return (delta < 0) ? (int8_t)(-pct) : (int8_t)pct;
  }
};
