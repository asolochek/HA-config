#pragma once

#include "Adafruit_seesaw.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace adafruit_soil_sensor {

struct TemperatureCalibration {
    float slope;
    float offset;
};

struct MoistureCalibration {
    uint16_t dry;
    uint16_t wet;
};

class AdafruitSoilSensorComponent : public PollingComponent, public i2c::I2CDevice {
  public:
    void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
    void set_moisture_sensor(sensor::Sensor *moisture_sensor) { moisture_sensor_ = moisture_sensor; }
    void set_temperature_calibration(float slope, float offset) {
        temperature_calibration.slope = slope;
        temperature_calibration.offset = offset;
    }
    void set_temperature_offset(float offset) { temperature_calibration.offset = offset; }
    void set_temperature_slope(float slope) { temperature_calibration.slope = slope; }
    void set_moisture_calibration(uint16_t dry, uint16_t wet) {
        moisture_calibration.dry = dry;
        moisture_calibration.wet = wet;
    }
    void set_moisture_dry(uint16_t dry) { moisture_calibration.dry = dry; }
    void set_moisture_wet(uint16_t wet) { moisture_calibration.wet = wet; }

    void setup() override;
    void update() override;
    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::DATA; }

    TemperatureCalibration temperature_calibration;
    MoistureCalibration moisture_calibration;

  protected:
    sensor::Sensor *temperature_sensor_{nullptr};
    sensor::Sensor *moisture_sensor_{nullptr};

    float get_temperature_();
    uint16_t get_moisture_();
};

}  // namespace adafruit_soil_sensor
}  // namespace esphome