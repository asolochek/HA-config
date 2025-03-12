#include "hdc302x.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace hdc302x {

static const char *const TAG = "hdc302x";

static const uint8_t HDC302X_ADDRESS = 0x44;  // Configurable to 0x45, 0x46, and 0x47

static const uint8_t HDC302X_CMD_ON_DEMAND_LPM0[2] = {0x24, 0x00};
static const uint8_t HDC302X_CMD_ON_DEMAND_LPM1[2] = {0x24, 0x0B};
static const uint8_t HDC302X_CMD_ON_DEMAND_LPM2[2] = {0x24, 0x16};
static const uint8_t HDC302X_CMD_ON_DEMAND_LPM3[2] = {0x24, 0xFF};
static const uint8_t HDC302X_CMD_AUTO_2SEC_LPM0[2] = {0x20, 0x32};
static const uint8_t HDC302X_CMD_AUTO_2SEC_LPM1[2] = {0x20, 0x24};
static const uint8_t HDC302X_CMD_AUTO_2SEC_LPM2[2] = {0x20, 0x2F};
static const uint8_t HDC302X_CMD_AUTO_2SEC_LPM3[2] = {0x20, 0xFF};
static const uint8_t HDC302X_CMD_AUTO_1SEC_LPM0[2] = {0x21, 0x30};
static const uint8_t HDC302X_CMD_AUTO_1SEC_LPM1[2] = {0x21, 0x26};
static const uint8_t HDC302X_CMD_AUTO_1SEC_LPM2[2] = {0x21, 0x2D};
static const uint8_t HDC302X_CMD_AUTO_1SEC_LPM3[2] = {0x21, 0xFF};
static const uint8_t HDC302X_CMD_AUTO_500MSEC_LPM0[2] = {0x22, 0x36};
static const uint8_t HDC302X_CMD_AUTO_500MSEC_LPM1[2] = {0x22, 0x20};
static const uint8_t HDC302X_CMD_AUTO_500MSEC_LPM2[2] = {0x22, 0x2B};
static const uint8_t HDC302X_CMD_AUTO_500MSEC_LPM3[2] = {0x22, 0xFF};
static const uint8_t HDC302X_CMD_AUTO_250MSEC_LPM0[2] = {0x23, 0x34};
static const uint8_t HDC302X_CMD_AUTO_250MSEC_LPM1[2] = {0x23, 0x22};
static const uint8_t HDC302X_CMD_AUTO_250MSEC_LPM2[2] = {0x23, 0x2B};
static const uint8_t HDC302X_CMD_AUTO_250MSEC_LPM3[2] = {0x23, 0xFF};
static const uint8_t HDC302X_CMD_AUTO_100MSEC_LPM0[2] = {0x27, 0x37};
static const uint8_t HDC302X_CMD_AUTO_100MSEC_LPM1[2] = {0x27, 0x21};
static const uint8_t HDC302X_CMD_AUTO_100MSEC_LPM2[2] = {0x27, 0x2A};
static const uint8_t HDC302X_CMD_AUTO_100MSEC_LPM3[2] = {0x27, 0xFF};
static const uint8_t HDC302X_CMD_EXIT_AUTO_MODE[2] = {0x30, 0x93};
static const uint8_t HDC302X_CMD_READ_MEASUREMENTS[2] = {0xE0, 0x00};
static const uint8_t HDC302X_CMD_READ_RH_MEASUREMENT[2] = {0xE0, 0x01};
static const uint8_t HDC302X_CMD_READ_MIN_TEMPERATURE[2] = {0xE0, 0x02};
static const uint8_t HDC302X_CMD_READ_MAX_TEMPERATURE[2] = {0xE0, 0x03};
static const uint8_t HDC302X_CMD_READ_MIN_RH[2] = {0xE0, 0x04};
static const uint8_t HDC302X_CMD_READ_MAX_RH[2] = {0xE0, 0x05};
static const uint8_t HDC302X_CMD_SET_LOW_ALERT_THRESHOLD[2] = {0x61, 0x00};
static const uint8_t HDC302X_CMD_SET_HIGH_ALERT_THRESHOLD[2] = {0x61, 0x1D};
static const uint8_t HDC302X_CMD_SET_LOW_CLEAR_THRESHOLD[2] = {0x61, 0x0B};
static const uint8_t HDC302X_CMD_SET_HIGH_CLEAR_THRESHOLD[2] = {0x61, 0x16};
static const uint8_t HDC302X_CMD_READ_LOW_ALERT_THRESHOLD[2] = {0xE1, 0x02};
static const uint8_t HDC302X_CMD_READ_HIGH_ALERT_THRESHOLD[2] = {0xE1, 0x1F};
static const uint8_t HDC302X_CMD_READ_LOW_CLEAR_THRESHOLD[2] = {0xE1, 0x09};
static const uint8_t HDC302X_CMD_READ_HIGH_CLEAR_THRESHOLD[2] = {0xE1, 0x14};
static const uint8_t HDC302X_CMD_ENABLE_HEATER[2] = {0x30, 0x6D};
static const uint8_t HDC302X_CMD_DISABLE_HEATER[2] = {0x30, 0x66};
static const uint8_t HDC302X_CMD_CONFIGURE_AND_READ_BACK_HEATER[2] = {0x30, 0x6E};
static const uint8_t HDC302X_CMD_READ_STATUS[2] = {0xF3, 0x2D};
static const uint8_t HDC302X_CMD_CLEAR_STATUS[2] = {0x30, 0x41};
static const uint8_t HDC302X_CMD_SOFT_RESET[2] = {0x30, 0xA2};
static const uint8_t HDC302X_CMD_READ_NIST_ID_BYTE4_5[2] = {0x36, 0x83};
static const uint8_t HDC302X_CMD_READ_NIST_ID_BYTE3_2[2] = {0x36, 0x84};
static const uint8_t HDC302X_CMD_READ_NIST_ID_BYTE1_0[2] = {0x36, 0x85};
static const uint8_t HDC302X_CMD_READ_MANUFACTURER_ID[2] = {0x37, 0x81};  // Should return 0x3000

// Programming the EEPROM requires the device be in sleep mode, and after each command
// there must be a 77ms delay before the next command can be issued.
static const uint8_t HDC302X_CMD_SAVE_THRESHOLDS_TO_NVM[2] = {0x61, 0x55};
static const uint8_t HDC302X_CMD_PROGRAM_OFFSETS[2] = {0xA0, 0x04};
static const uint8_t HDC302X_CMD_PROGRAM_CONFIG[2] = {0x61, 0xBB};

static const uint8_t HDC302X_CFG_AUTO_2SEC_LPM0[2] = {0x00, 0x03};
static const uint8_t HDC302X_CFG_AUTO_2SEC_LPM1[2] = {0x00, 0x13};
static const uint8_t HDC302X_CFG_AUTO_2SEC_LPM2[2] = {0x00, 0x23};
static const uint8_t HDC302X_CFG_AUTO_2SEC_LPM3[2] = {0x00, 0x33};
static const uint8_t HDC302X_CFG_AUTO_1SEC_LPM0[2] = {0x00, 0x05};
static const uint8_t HDC302X_CFG_AUTO_1SEC_LPM1[2] = {0x00, 0x15};
static const uint8_t HDC302X_CFG_AUTO_1SEC_LPM2[2] = {0x00, 0x25};
static const uint8_t HDC302X_CFG_AUTO_1SEC_LPM3[2] = {0x00, 0x35};
static const uint8_t HDC302X_CFG_AUTO_500MSEC_LPM0[2] = {0x00, 0x07};
static const uint8_t HDC302X_CFG_AUTO_500MSEC_LPM1[2] = {0x00, 0x17};
static const uint8_t HDC302X_CFG_AUTO_500MSEC_LPM2[2] = {0x00, 0x27};
static const uint8_t HDC302X_CFG_AUTO_500MSEC_LPM3[2] = {0x00, 0x37};
static const uint8_t HDC302X_CFG_AUTO_250MSEC_LPM0[2] = {0x00, 0x09};
static const uint8_t HDC302X_CFG_AUTO_250MSEC_LPM1[2] = {0x00, 0x19};
static const uint8_t HDC302X_CFG_AUTO_250MSEC_LPM2[2] = {0x00, 0x29};
static const uint8_t HDC302X_CFG_AUTO_250MSEC_LPM3[2] = {0x00, 0x39};
static const uint8_t HDC302X_CFG_AUTO_100MSEC_LPM0[2] = {0x00, 0x0B};
static const uint8_t HDC302X_CFG_AUTO_100MSEC_LPM1[2] = {0x00, 0x1B};
static const uint8_t HDC302X_CFG_AUTO_100MSEC_LPM2[2] = {0x00, 0x2B};
static const uint8_t HDC302X_CFG_AUTO_100MSEC_LPM3[2] = {0x00, 0x3B};
static const uint8_t HDC302X_CFG_RESTORE_DEFAULTS[2] = {0x00, 0x00};

uint8_t HDC302XComponent::crcHDC3(const uint8_t data[2]) {
  return crcHDC3(data[0] << 8 | data[1]);
}

uint8_t HDC302XComponent::crcHDC3(uint16_t data) {
  uint8_t crc = 0xFF;
  uint8_t msg[2] = {(uint8_t) (data >> 8), (uint8_t) (data & 0xFF)};

  for (int byte = 0; byte < 2; byte++) {
    crc ^= msg[byte];
    for (int bit = 0; bit < 8; bit++) {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x31;
      else
        crc = (crc << 1);
    }
  }
  return crc;
}

bool HDC302XComponent::send_command(const uint8_t cmd[]) { 
  return this->write(cmd, 2);
}

bool HDC302XComponent::send_config(const uint8_t cmd[], const uint8_t cfg[]) {
  uint8_t crc = crcHDC3(cfg[0] << 8 | cfg[1]);
  uint8_t data[4] = {cmd[1], cfg[0], cfg[1], crc};

  return this->write_bytes(cmd[0], data, 4);
}

void HDC302XComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HDC302X...");

  uint8_t buf[3];

  send_command(HDC302X_CMD_READ_MANUFACTURER_ID);
  uint8_t expected_crc = crcHDC3(HDC302X_CMD_READ_MANUFACTURER_ID);
  this->read(buf, 3);
  if (buf[2] != expected_crc) {
    ESP_LOGW(TAG, "HDC302X CRC error");
    this->status_set_warning();
    return;
  }

  if (buf[0] != 0x30 || buf[1] != 0x00) {
    ESP_LOGW(TAG, "HDC302X manufacturer ID mismatch");
    this->status_set_warning();
    return;
  }

  send_command(HDC302X_CMD_CLEAR_STATUS);

  // If we are in auto-mode, we need to exit it first
  if (send_command(HDC302X_CMD_EXIT_AUTO_MODE) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "HDC302X exit auto mode instruction error");
    this->status_set_warning();
    return;
  }

  if (this->_heater_enable) {
    if (send_command(HDC302X_CMD_ENABLE_HEATER) != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "HDC302X enable heater instruction error");
      this->status_set_warning();
      return;
    }
  } else {
    if (send_command(HDC302X_CMD_DISABLE_HEATER) != i2c::ERROR_OK) {
      ESP_LOGW(TAG, "HDC302X disable heater instruction error");
      this->status_set_warning();
      return;
    }
  }

  // Setup for auto-mode, 1 second, low power mode 0
  // if (!send_config(HDC302X_CMD_PROGRAM_CONFIG, HDC302X_CFG_AUTO_1SEC_LPM0)) {
  //   ESP_LOGW(TAG, "HDC302X initial config instruction error");
  //   this->status_set_warning();
  //   return;
  // }
  if (send_command(HDC302X_CMD_AUTO_1SEC_LPM0) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "HDC302X initial config instruction error");
    this->status_set_warning();
    return;
  }

}

void HDC302XComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "HDC302X:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with HDC302X failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
  ESP_LOGCONFIG(TAG, "  Heater %s", this->_heater_enable ? "enabled" : "disabled");
}

void HDC302XComponent::update() {
  uint8_t buf[6];
  uint16_t raw_temp;
  uint16_t raw_humidity;
  uint8_t ret;

  // if (ret = send_command(HDC302X_CMD_READ_MEASUREMENTS) != i2c::ERROR_OK) {
  //   ESP_LOGW(TAG, "HDC302X read measurements instruction error: %.2X", ret);
  //   this->status_set_warning();
  //   return;
  // }

  if (ret = send_command(HDC302X_CMD_ON_DEMAND_LPM0) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "HDC302X read measurements instruction error: %.2X", ret);
    this->status_set_warning();
    return;
  }

  delay(20);

  if (ret = this->read(buf, 6) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "HDC302X read measurements error: %.2X", ret);
    this->status_set_warning();
    return;
  }

  raw_temp = buf[0] << 8 | buf[1];
  raw_humidity = buf[3] << 8 | buf[4];

  uint8_t temp_crc = crcHDC3(raw_temp);
  uint8_t humidity_crc = crcHDC3(raw_humidity);

  if (temp_crc != buf[2] || humidity_crc != buf[5]) {
    ESP_LOGW(TAG, "HDC302X CRC error");
    this->status_set_warning();
    return;
  }

  float temp = ((raw_temp / 65535.0f) * 175.0) - 45.0f;
  this->temperature_->publish_state(temp);

  float humidity = (raw_humidity / 65535.0f) * 100.0;
  this->humidity_->publish_state(humidity);

  ESP_LOGVV(TAG, "Got temperature=%.1f°C humidity=%.1f%%", temp, humidity);
  this->status_clear_warning();
}
float HDC302XComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace hdc302x
}  // namespace esphome
