
/* ==================== RPM COUNTER ==================== */
class RPMCounter {
  uint32_t lastCalc = 0;
  uint16_t rpm = 0;
  pcnt_unit_handle_t pcnt_unit = NULL;
  pcnt_channel_handle_t pcnt_chan = NULL;

public:
  void begin() {
    pcnt_unit_config_t unit_config = {
      .low_limit = -4000,
      .high_limit = 4000,
      .flags = {}
    };
    pcnt_new_unit(&unit_config, &pcnt_unit);
    pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = 5000 };
    pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);
    pcnt_chan_config_t chan_config = {
      .edge_gpio_num = PIN_RPM_HALL,
      .level_gpio_num = -1,
      .flags = {}
    };
    pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan);
    pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
    pcnt_channel_set_level_action(pcnt_chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_KEEP);
    pcnt_unit_enable(pcnt_unit);
    pcnt_unit_clear_count(pcnt_unit);
    pcnt_unit_start(pcnt_unit);
  }

  void update() {
    if (millis() - lastCalc < 100) return;
    if (!pcnt_unit) return;
    int count = 0;
    if (pcnt_unit_get_count(pcnt_unit, &count) == ESP_OK) {
      pcnt_unit_clear_count(pcnt_unit);
      rpm = abs(count) * 60000UL / 100;
    }
    lastCalc = millis();
  }
  uint16_t getRPM() const { return rpm; }
};
