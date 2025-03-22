#include "adafruit_soil_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace adafruit_soil_sensor {

static const char *const TAG = "adafruit_soil_binary_sensor";

void AdafruitSoilBinarySensorComponent::setup() {
  float state = this->moisture_sensor_->get_state();

  if (std::isnan(state)) {
    this->publish_initial_state(false);
  } else {
    this->publish_initial_state(state < this->threshold_);
  }
}

void AdafruitSoilBinarySensorComponent::set_sensor(sensor::Sensor *moisture_sensor) {
  this->moisture_sensor_ = moisture_sensor;

  this->moisture_sensor_->add_on_raw_state_callback([this](float state) {
    ESP_LOGVV(TAG, "Callback %s: %.2f", this->moisture_sensor_->get_name(), state);
    if (!std::isnan(state)) {
      if (state < this->threshold_) {
        this->publish_state(true);
        this->is_on_ = true;
      } else if (state > this->threshold_ + this->hysteresis_) {
        this->publish_state(false);
        this->is_on_ = false;
      }
    }  
  });
}


void AdafruitSoilBinarySensorComponent::dump_config() {
  LOG_BINARY_SENSOR(TAG, "  Adafruit Soil Binary Sensor: ", this);
  LOG_SENSOR(TAG, "  Moisture Sensor", this->moisture_sensor_);
  ESP_LOGCONFIG(TAG, "  Threshold: %.2f", this->threshold_);
  ESP_LOGCONFIG(TAG, "  Hysteresis: %.2f", this->hysteresis_);
}

} // namespace adafruit_soil_sensor
} // namespace esphome