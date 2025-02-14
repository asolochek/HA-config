#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace hdc302x {

typedef struct {
  uint16_t alert : 1;
  uint16_t rsvd : 1;
  uint16_t heater : 1;
  uint16_t rsvd2 : 1;
  uint16_t rh_alert : 1;
  uint16_t temp_alert : 1;
  uint16_t rh_high_alert : 1;
  uint16_t rh_low_alert : 1;
  uint16_t temp_high_alert : 1;
  uint16_t temp_low_alert : 1;
  uint16_t rsvd3 : 1;
  uint16_t reset_detected : 1;
  uint16_t rsvd4 : 3;
  uint16_t checksum_valid : 1;
} hdc302x_status_t;


class HDC302XComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_temperature(sensor::Sensor *temperature) { temperature_ = temperature; }
  void set_humidity(sensor::Sensor *humidity) { humidity_ = humidity; }
  void set_heater_enable(bool enable) { _heater_enable = enable; }

  /// Setup the sensor and check for connection.
  void setup() override;
  void dump_config() override;
  /// Retrieve the latest sensor values. This operation takes approximately 16ms.
  void update() override;

  float get_setup_priority() const override;

 protected:
  sensor::Sensor *temperature_{nullptr};
  sensor::Sensor *humidity_{nullptr};
  bool _heater_enable;

 private:
  uint8_t crcHDC3(uint16_t data);
  uint8_t crcHDC3(const uint8_t data[2]);
  bool send_command(const uint8_t cmd[]);
  bool send_config(const uint8_t cmd[], const uint8_t cfg[]);
};

}  // namespace hdc302x
}  // namespace esphome
