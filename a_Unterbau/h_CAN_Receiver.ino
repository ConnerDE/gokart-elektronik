
/* ==================== CAN RECEIVER ==================== */
class CanReceiver {
  unsigned long lastFrame = 0;
  uint8_t lastCounter = 0;
  unsigned long lastCounterChange = 0;

public:
  uint8_t exhaustAngle = 0;
  uint8_t dashPage = 0;
  uint8_t driveMode = 0;
  uint8_t lightMode = 0;
  uint8_t rgbMode = 0;
  uint16_t buttons = 0xFFFF;
  bool dataValid = false;

  void begin() {
    twai_general_config_t g_config = {
      .mode = TWAI_MODE_NORMAL,
      .tx_io = (gpio_num_t)PIN_CAN_TX,
      .rx_io = (gpio_num_t)PIN_CAN_RX,
      .clkout_io = TWAI_IO_UNUSED,
      .bus_off_io = TWAI_IO_UNUSED,
      .tx_queue_len = 5,
      .rx_queue_len = 5,
      .alerts_enabled = TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED,
      .clkout_divider = 0,
      .intr_flags = ESP_INTR_FLAG_LEVEL1
    };
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_driver_install(&g_config, &t_config, &f_config);
    twai_start();
  }

  void update() {
    uint32_t alerts;
    if (twai_read_alerts(&alerts, 0) == ESP_OK) {
      if (alerts & TWAI_ALERT_BUS_OFF) twai_initiate_recovery();
      if (alerts & TWAI_ALERT_BUS_RECOVERED) twai_start();
    }

    twai_message_t message;
    while (twai_receive(&message, 0) == ESP_OK) {
      if (message.data_length_code >= 8) {
        exhaustAngle = message.data[0];
        dashPage = message.data[1];
        driveMode = message.data[2];
        lightMode = message.data[3];
        rgbMode = message.data[4];
        buttons = message.data[5] | (message.data[6] << 8);
        uint8_t currentCounter = message.data[7];
        if (currentCounter != lastCounter) {
          lastCounter = currentCounter;
          lastCounterChange = millis();
        }
        dataValid = true;
        lastFrame = millis();
      }
    }
    if (millis() - lastFrame > CAN_TIMEOUT_MS) dataValid = false;
    if (millis() - lastCounterChange > 500) dataValid = false;
  }

  bool isBlinkRight()   { return !(buttons & (1 << 0)); }
  bool isShiftDown()    { return !(buttons & (1 << 1)); }
  bool isDRS()          { return !(buttons & (1 << 2)); }  // !!!
  bool isReverse()      { return !(buttons & (1 << 3)); }
  bool isBlinkLeft()    { return !(buttons & (1 << 4)); }
  bool isShiftUp()      { return !(buttons & (1 << 5)); }
  bool isWarnBlink()    { return !(buttons & (1 << 7)); }
  bool isLaunch()       { return !(buttons & (1 << 9)); }
  bool isHorn()         { return !(buttons & (1 << 15)); }
};
