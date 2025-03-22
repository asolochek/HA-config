#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace adafruit_soil_sensor {


class AdafruitSoilBinarySensorComponent : public Component, public binary_sensor::BinarySensor {
 public:
  void setup() override;

  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_sensor(sensor::Sensor *moisture_sensor);
  void set_threshold(float threshold) { this->threshold_ = threshold; }
  void set_hysteresis(float hysteresis) { this->hysteresis_ = hysteresis; }

 protected:
  sensor::Sensor *moisture_sensor_{nullptr};

  float threshold_;
  float hysteresis_;
  bool is_on_{false};
  
};

}  // namespace adafruit_soil_sensor
}  // namespace esphome