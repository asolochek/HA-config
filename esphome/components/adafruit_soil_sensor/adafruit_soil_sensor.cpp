#include "esphome.h"
#include "adafruit_soil_sensor.h"
#include "esphome/core/log.h"
#include "esphome/components/climate_utils/climate_utils.h"

namespace esphome {
namespace adafruit_soil_sensor {

static const char *const TAG = "adafruit_soil_sensor";

void AdafruitSoilSensorComponent::setup() 
{
  uint8_t buf[4] = {SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, 0xFF};
//  i2c::I2CRegister16 reg_hw_id = this->reg16(SEESAW_STATUS_BASE | SEESAW_STATUS_HW_ID);

  this->write(buf, 3);
  delayMicroseconds(10000);

//  this->read_register16(SEESAW_STATUS_BASE << 8 | SEESAW_STATUS_HW_ID, buf, 1);

  buf[1] = SEESAW_STATUS_HW_ID;
  this->write(buf, 2);
  this->read_bytes_raw(buf, 1);

  //  if (reg_hw_id.get() != SEESAW_HW_ID_CODE_SAMD09) {
  if (buf[0] != SEESAW_HW_ID_CODE_SAMD09) {
    ESP_LOGE("soil_sensor", "Failed to connect to soil sensor.");
    this->mark_failed();
    return;
  }

  ESP_LOGI("soil_sensor", "Successfully reset soil sensor.");

  this->temperature_sensor_->set_unit_of_measurement("°F");
}

void AdafruitSoilSensorComponent::update() 
{
  static float previous_temp = 0;
  static uint16_t previous_moisture = 0;
  float temp_c = this->get_temperature_();
  uint16_t moisture = this->get_moisture_();
  float moisture_pct = 0;

//  ESP_LOGD(TAG, "%s: %f", this->temperature_sensor_->get_name(), temp_c);
//  ESP_LOGD(TAG, "%s: %d", this->moisture_sensor_->get_name(), moisture);

  float temp_f = c_to_f(temp_c);

  temp_f = temp_f * this->temperature_calibration.slope + this->temperature_calibration.offset;

  float temp = roundf(temp_f * 10.0);
  temp_f = temp / 10.0;

//  moisture_pct = map(moisture, this->moisture_calibration.dry, this->moisture_calibration.wet, 0, 100);

  moisture_pct = ((float)moisture - (float)this->moisture_calibration.dry) / ((float)this->moisture_calibration.wet - (float)this->moisture_calibration.dry) * 100.0;
  ESP_LOGVV(TAG, "Moisture Raw: %d (pct: %.2f)", moisture, moisture_pct);

  if (temp_f != previous_temp) {
    previous_temp = temp_f;
    this->temperature_sensor_->publish_state(temp_f);
  }

  // ESP_LOGVV(TAG, "Moisture Raw: %d (%d)", moisture, previous_moisture);
  // if (moisture != previous_moisture) {
  //   previous_moisture = moisture;
    this->moisture_sensor_->publish_state(moisture_pct);
  // }
}

float AdafruitSoilSensorComponent::get_temperature_() 
{
  uint8_t buf[4] = { SEESAW_STATUS_BASE, SEESAW_STATUS_TEMP, 0, 0};

  this->write(buf, 2);
  delayMicroseconds(5000);
  this->read(buf, 4);

  // this->read_register16(SEESAW_STATUS_BASE << 8 | SEESAW_STATUS_TEMP, buf, 4);

  int32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
      ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];


  return (1.0 / (1UL << 16)) * ret;
}

uint16_t AdafruitSoilSensorComponent::get_moisture_()
{
  uint8_t buf[2];
  uint16_t ret = 65535;
  uint8_t reg[2] = {SEESAW_TOUCH_BASE, SEESAW_TOUCH_CHANNEL_OFFSET};

  do {
    delay(1);

    // Reading the moisture (capacitive touch) sensor requires a delay between the write and read,
    // so we can't use the read_register16 method.
    this->write(reg, 2);
    delayMicroseconds(5000);
    this->read(buf, 2);
    ESP_LOGVV(TAG, "Moisture Raw: 0x%.2X 0x%.2X", buf[0], buf[1]);
    ret = ((uint16_t)buf[0] << 8) | buf[1];
  } while (ret == 65535);

//  ESP_LOGVV(TAG, "Moisture Raw: %d", ret);
  return ret;
}

void AdafruitSoilSensorComponent::dump_config() 
{
  ESP_LOGCONFIG(TAG, "Adafruit Soil Sensor:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Temperature Sensor", this->temperature_sensor_);
  LOG_SENSOR("  ", "Moisture Sensor", this->moisture_sensor_);
}

}  // namespace adafruit_soil_sensor
} // namespace esphome
