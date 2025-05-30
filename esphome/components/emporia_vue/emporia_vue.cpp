#include "emporia_vue.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emporia_vue {

static const char *const TAG = "emporia_vue";

void EmporiaVueComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Emporia Vue");
  LOG_I2C_DEVICE(this);

  for (auto *phase : this->phases_) {
    std::string wire;
    switch (phase->get_input_wire()) {
      case PhaseInputWire::BLACK:
        wire = "BLACK";
        break;
      case PhaseInputWire::RED:
        wire = "RED";
        break;
      case PhaseInputWire::BLUE:
        wire = "BLUE";
        break;
    }
    ESP_LOGCONFIG(TAG, "  Phase");
    ESP_LOGCONFIG(TAG, "    Wire: %s", wire.c_str());
    ESP_LOGCONFIG(TAG, "    Calibration: %f", phase->get_calibration());
    LOG_SENSOR("    ", "Voltage", phase->get_voltage_sensor());
  }

  for (auto *ct_clamp : this->ct_clamps_) {
    ESP_LOGCONFIG(TAG, "  CT Clamp");
    ESP_LOGCONFIG(TAG, "    Phase Calibration: %f", ct_clamp->get_phase()->get_calibration());
    ESP_LOGCONFIG(TAG, "    CT Port Index: %d", ct_clamp->get_input_port());
    LOG_SENSOR("    ", "Power", ct_clamp->get_power_sensor());
    LOG_SENSOR("    ", "Current", ct_clamp->get_current_sensor());
  }
}

void EmporiaVueComponent::update() {
  SensorReading sensor_reading;

  uint8_t buf[1424] = {{0}};

  i2c::ErrorCode err = read(reinterpret_cast<uint8_t *>(&sensor_reading), sizeof(sensor_reading));

  if (err != i2c::ErrorCode::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read from sensor due to I2C error %d", err);
    return;
  }

  if (sensor_reading.end != 0) {
    ESP_LOGE(TAG, "Failed to read from sensor due to a malformed reading, should end in null bytes but is %d",
             sensor_reading.end);
    return;
  }

  if (!sensor_reading.is_unread) {
    ESP_LOGV(TAG, "Ignoring sensor reading that is marked as read");
    return;
  }



  // ESP_LOGI(TAG, "New Packet Received: Sequence #%d, Checksum %d, Unread %s, Current[2]: %ld, Current[3]: %ld",
  //          sensor_reading.sequence_num, sensor_reading.checksum,
  //          sensor_reading.is_unread ? "true" : "false",
  //          sensor_reading.current[2], sensor_reading.current[3]);

//   for (int i = 0; i < sizeof(sensor_reading); i++) {
//     sprintf((char*)(buf+3*i), "%02X ", ((uint8_t *)&sensor_reading)[i]);
// //    printf("%02X ", ((uint8_t *)&sensor_reading)[i]);
//   }
//   ESP_LOGD(TAG, "%s", buf);

  // for(int i=0; i < 19; i++) {
  //   sprintf((char*)(buf+7*i), "0x%04lX ", sensor_reading.current[i]);

  //   // ESP_LOGD(TAG, "Power[%d]: black=%d, red=%d, blue=%d", i, sensor_reading.power[i].phase_black,
  //   //          sensor_reading.power[i].phase_red, sensor_reading.power[i].phase_blue);
  //   // ESP_LOGD(TAG, "Current[%d]: %d", i, sensor_reading.current[i]);
  // }
  // ESP_LOGD(TAG, "%s", buf);




  if (this->last_sequence_num_ && sensor_reading.sequence_num > this->last_sequence_num_ + 1) {
    ESP_LOGW(TAG, "Detected %d missing reading(s), data may not be accurate!",
             sensor_reading.sequence_num - this->last_sequence_num_ - 1);
  }

  for (auto *phase : this->phases_) {
    phase->update_from_reading(sensor_reading);
  }
  for (auto *ct_clamp : this->ct_clamps_) {
    ct_clamp->update_from_reading(sensor_reading);
  }

  this->last_sequence_num_ = sensor_reading.sequence_num;

  this->callback_.call();
}

void EmporiaVueComponent::add_on_update_callback(std::function<void()> &&callback) {
  this->callback_.add(std::move(callback));
}

void PhaseConfig::update_from_reading(const SensorReading &sensor_reading) {
  if (this->voltage_sensor_) {
    float calibrated_voltage = sensor_reading.voltage[this->input_wire_] * this->calibration_;
    this->voltage_sensor_->publish_state(calibrated_voltage);
  }

  uint16_t raw_frequency = sensor_reading.frequency;
  // validation that these sensors are allowed on this phase is done in the codegen stage
  if (this->frequency_sensor_) {
    // see https://github.com/emporia-vue-local/esphome/pull/88 for constant explanation
    float frequency = 19610.0f / (float) raw_frequency;
    this->frequency_sensor_->publish_state(frequency);
  }
  if (this->phase_angle_sensor_) {
    uint16_t raw_phase_angle = sensor_reading.degrees[((uint8_t) this->input_wire_) - 1];
    float phase_angle = ((float) raw_phase_angle) * 360.0f / ((float) raw_frequency);
    // this is truncated to a uint16_t on the vue 2
    this->phase_angle_sensor_->publish_state(phase_angle);
  }
}

int32_t PhaseConfig::extract_power_for_phase(const ReadingPowerEntry &power_entry) {
  switch (this->input_wire_) {
    case PhaseInputWire::BLACK:
      return power_entry.phase_black;
    case PhaseInputWire::RED:
      return power_entry.phase_red;
    case PhaseInputWire::BLUE:
      return power_entry.phase_blue;
    default:
      ESP_LOGE(TAG, "Unsupported phase input wire, this should never happen");
      return -1;
  }
}

void CTClampConfig::update_from_reading(const SensorReading &sensor_reading) {
  if (this->power_sensor_) {
    ReadingPowerEntry power_entry = sensor_reading.power[this->input_port_];
    int32_t raw_power = this->phase_->extract_power_for_phase(power_entry);
    float calibrated_power = this->get_calibrated_power(raw_power);
    this->power_sensor_->publish_state(calibrated_power);
  }
  if (this->current_sensor_) {
    uint16_t raw_current = sensor_reading.current[this->input_port_];
    double raw_current_d = (double) raw_current;
    double scalar;
    double offset = 0.0;
    if (raw_current_d <= 50) {
      raw_current_d = 0;
    }
    if (this->input_port_ <= CTInputPort::A) {
      scalar = 775.0 / 42624.0;
      // scalar = 0.0191;
      // offset = -0.2167;
    } else {
      scalar = 0.004687;
      offset = -0.0244;
      // scalar = 775.0 / 170496.0;
    }
    this->current_sensor_->publish_state(raw_current_d * scalar + offset);
  }
}

float CTClampConfig::get_calibrated_power(int32_t raw_power) const {
  float calibration = this->phase_->get_calibration();

  float correction_factor = (this->input_port_ < 3) ? 5.5 : 22;

  return (raw_power * calibration) / correction_factor;
}

}  // namespace emporia_vue
}  // namespace esphome
