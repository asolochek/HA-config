#include "exhaust_fan.h"
#include <math.h>

namespace esphome {
namespace exhaust_fan {

static const char *const TAG = "exhaust_fan";


ExhaustFan::ExhaustFan()
    : idle_trigger_(new Trigger<>()), stage1_trigger_(new Trigger<>()), stage2_trigger_(new Trigger<>()) {}



void ExhaustFan::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Exhaust Fan...");
  this->temperature_sensor_->add_on_state_callback([this](float state) {
    DBG(ESP_LOGD(TAG, "Temperature Sensor State Update: %.1f", state));
    this->compute_state_();
  });

  this->humidity_sensor_->add_on_state_callback([this](float state) {
    DBG(ESP_LOGD(TAG, "Humidity Sensor State Update: %.1f", state));
    this->compute_state_();
  });

  this->external_temperature_sensor_->add_on_state_callback([this](float state) {
    DBG(ESP_LOGD(TAG, "External Temperature Sensor State Update: %.1f", state));
    this->compute_state_();
  });

  this->external_humidity_sensor_->add_on_state_callback([this](float state) {
    DBG(ESP_LOGD(TAG, "External Humidity Sensor State Update: %.1f", state));
    this->compute_state_();
  });

  /// TODO: Optionally Restore values here

  this->update();
}


void ExhaustFan::dump_config() {
  ESP_LOGCONFIG(TAG, "Exhaust Fan:");
  ESP_LOGCONFIG(TAG, "  Temperature Sensor: %s", this->temperature_sensor_->get_name().c_str());
  ESP_LOGCONFIG(TAG, "  Humidity Sensor: %s", this->humidity_sensor_->get_name().c_str());
  ESP_LOGCONFIG(TAG, "  External Temperature Sensor: %s", this->external_temperature_sensor_->get_name().c_str());
  ESP_LOGCONFIG(TAG, "  External Humidity Sensor: %s", this->external_humidity_sensor_->get_name().c_str());
  ESP_LOGCONFIG(TAG, "  Temperature: %.1f°F", c_to_f(this->get_temperature()));
  ESP_LOGCONFIG(TAG, "  Humidity: %.1f%%", this->get_humidity());
  ESP_LOGCONFIG(TAG, "  Dewpoint: %.1f°F", c_to_f(this->get_dewpoint()));
  ESP_LOGCONFIG(TAG, "  External Temperature: %.1f°F", c_to_f(this->get_external_temperature()));
  ESP_LOGCONFIG(TAG, "  External Humidity: %.1f%%", this->get_external_humidity());
  ESP_LOGCONFIG(TAG, "  External Dewpoint: %.1f°F", c_to_f(this->get_external_dewpoint()));
  ESP_LOGCONFIG(TAG, "  Stage 1 Clear: %.1f%% (%.1f°F)", this->get_stage1_clear(), c_to_f(this->get_stage1_clear_dp()));
  ESP_LOGCONFIG(TAG, "  Stage 1 Trip: %.1f%% (%.1f°F)", this->get_stage1_trip(), c_to_f(this->get_stage1_trip_dp()));
  ESP_LOGCONFIG(TAG, "  Stage 2 Clear: %.1f%% (%.1f°F)", this->get_stage2_clear(), c_to_f(this->get_stage2_clear_dp()));
  ESP_LOGCONFIG(TAG, "  Stage 2 Trip: %.1f%% (%.1f°F)", this->get_stage2_trip(), c_to_f(this->get_stage2_trip_dp()));
  ESP_LOGCONFIG(TAG, "  Baseline Offset: %.1f", this->get_baseline_offset());
  ESP_LOGCONFIG(TAG, "  Hysteresis: %.1f", this->get_hysteresis());
  ESP_LOGCONFIG(TAG, "  Min RH: %.1f%%", this->get_min_humidity());
}

void ExhaustFan::set_temperature_sensor(sensor::Sensor *temperature_sensor) {
  this->temperature_sensor_ = temperature_sensor;
}

void ExhaustFan::set_humidity_sensor(sensor::Sensor *humidity_sensor) {
  this->humidity_sensor_ = humidity_sensor;
}

void ExhaustFan::set_external_temperature_sensor(sensor::Sensor *external_temperature_sensor) {
  this->external_temperature_sensor_ = external_temperature_sensor;
}

void ExhaustFan::set_external_humidity_sensor(sensor::Sensor *external_humidity_sensor) {
  this->external_humidity_sensor_ = external_humidity_sensor;
}

void ExhaustFan::set_stage1_trip(float stage1_trip) {
  if (stage1_trip > this->stage2_trip_) {
    ESP_LOGW(TAG, "Stage 1 Trip must be less than Stage 2 Trip.");
  }
  this->stage1_trip_ = stage1_trip;
  ESP_LOGI(TAG, "Stage 1 Trip: %.1f%%", this->stage1_trip_);

  DBG(this->dump_config());
  this->update();
}

void ExhaustFan::set_stage2_trip(float stage2_trip) {
  if (stage2_trip < this->stage1_trip_) {
    ESP_LOGW(TAG, "Stage 2 Trip must be greater than Stage 1 Trip.");
  }
  this->stage2_trip_ = stage2_trip;
  ESP_LOGI(TAG, "Stage 2 Trip: %.1f%%", this->stage2_trip_);

  DBG(this->dump_config());
  this->update();
}

void ExhaustFan::set_stage1_trip_dp(float stage1_trip_dp) {
  this->stage1_trip_dp_ = stage1_trip_dp;
}

void ExhaustFan::set_stage1_clear_dp(float stage1_clear_dp) {
  this->stage1_clear_dp_ = stage1_clear_dp;
}

void ExhaustFan::set_stage2_trip_dp(float stage2_trip_dp) {
  this->stage2_trip_dp_ = stage2_trip_dp;
}

void ExhaustFan::set_stage2_clear_dp(float stage2_clear_dp) {
  this->stage2_clear_dp_ = stage2_clear_dp;
}

void ExhaustFan::set_baseline_offset(float baseline_offset) {
  this->baseline_offset_ = baseline_offset;
  ESP_LOGI(TAG, "Baseline Offset: %.1f", this->baseline_offset_);

  this->dump_config();
  this->update();
}

void ExhaustFan::set_hysteresis(float hysteresis) {
  this->hysteresis_ = hysteresis;
  ESP_LOGI(TAG, "Hysteresis: %.1f", this->hysteresis_);

  this->dump_config();
  this->update();
}

void ExhaustFan::set_min_humidity(float min_rh) {
  this->min_rh_ = min_rh;
  ESP_LOGI(TAG, "Min RH: %.1f%%", this->min_rh_);
}

void ExhaustFan::enable_controller(void) { 
  controller_enabled_ = true; 
  this->dump_config();
  this->update();
}

void ExhaustFan::disable_controller(void) { 
  controller_enabled_ = false; 
}

float ExhaustFan::get_stage1_clear() {
  float ret = this->stage1_trip_ - this->hysteresis_;
  if (this->stage1_clear_dp_) {
    ret = dewpoint_to_rh(this->get_temperature(), this->stage1_clear_dp_.value());
  } else {
    // If we have external sensors
    if ((this->external_temperature_sensor_ != nullptr) && 
        (this->external_humidity_sensor_ != nullptr) &&
        this->baseline_offset_) {
      float baseline = correct_humidity(this->get_external_temperature(), this->get_external_humidity(), this->get_temperature());
      ret = baseline + this->baseline_offset_.value();
      ESP_LOGV(TAG, "Calculated Stage 1 Clear RH based on external sensors and baseline offset: %.1f%%", ret);
    }
    if (this->min_rh_) {
      ret = fmax(ret, this->min_rh_.value());
    }
  }
  return ret;
}

float ExhaustFan::get_stage1_trip() {
  float ret = this->stage1_trip_;
  if (this->stage1_trip_dp_) {
    ret = dewpoint_to_rh(this->get_temperature(), this->stage1_trip_dp_.value());
  }
  return ret;
}

float ExhaustFan::get_stage2_clear() {
  float ret = this->stage2_trip_ - this->hysteresis_;
  if (this->stage2_clear_dp_) {
    ret = dewpoint_to_rh(this->get_temperature(), this->stage2_clear_dp_.value());
  }
  return ret;
}

float ExhaustFan::get_stage2_trip() {
  float ret = this->stage2_trip_;
  if (this->stage2_trip_dp_) {
    ret = dewpoint_to_rh(this->get_temperature(), this->stage2_trip_dp_.value());
  }
  return ret;
}

/**
 * @brief Get the dewpoint for stage 1 clear
 * 
 * @details If stage1_clear_dp_ is set, return that value, otherwise calculate dewpoint
 *         based on temperature and stage1_clear() RH value.
 * 
 * @return float dewpoint for stage 1 clear
 */
float ExhaustFan::get_stage1_clear_dp() {
  return this->stage1_clear_dp_.value_or(rh_to_dewpoint(this->get_temperature(), this->get_stage1_clear()));
}

float ExhaustFan::get_stage1_trip_dp() {
  return this->stage1_trip_dp_.value_or(rh_to_dewpoint(this->get_temperature(), this->get_stage1_trip()));
}

float ExhaustFan::get_stage2_clear_dp() {
  return this->stage2_clear_dp_.value_or(rh_to_dewpoint(this->get_temperature(), this->get_stage2_clear()));
}

float ExhaustFan::get_stage2_trip_dp() {
  return this->stage2_trip_dp_.value_or(rh_to_dewpoint(this->get_temperature(), this->get_stage2_trip()));
}

float ExhaustFan::get_temperature() {
  float temp = this->temperature_sensor_->state;
  if (this->temperature_sensor_->get_unit_of_measurement_ref().str().back() == 'F') {
    return f_to_c(temp);
  } 
  return temp;
}

float ExhaustFan::get_humidity() {
  return this->humidity_sensor_->state;
}

float ExhaustFan::get_dewpoint() {
  return rh_to_dewpoint(this->get_temperature(), this->get_humidity());
}

float ExhaustFan::get_external_temperature() {
  float ret = NAN;
  if (this->external_temperature_sensor_ != nullptr) {
    ret = this->external_temperature_sensor_->state;

    if (this->external_temperature_sensor_->get_unit_of_measurement_ref().str().back() == 'F') {
      ret = f_to_c(ret);
    }
  }
  return ret;
}

float ExhaustFan::get_external_humidity() {
  float ret = NAN;
  if (this->external_humidity_sensor_ != nullptr) {
    ret = this->external_humidity_sensor_->state;
  }
  return ret;
}

float ExhaustFan::get_external_dewpoint() {
  float temp = this->get_external_temperature();
  float humidity = this->get_external_humidity();
  if (isnan(temp) || isnan(humidity)) {
    return NAN;
  }
  return rh_to_dewpoint(this->get_external_temperature(), this->get_external_humidity());
}

float ExhaustFan::get_baseline_offset() {
  return this->baseline_offset_.value_or(0);
}

float ExhaustFan::get_hysteresis() {
  return this->hysteresis_;
}

float ExhaustFan::get_min_humidity() {
  return this->min_rh_.value_or(0);
}

ExhaustFanZone ExhaustFan::get_current_zone_() {
  float dp = this->get_dewpoint();
  
  if (dp == NAN) {
    ESP_LOGW(TAG, "Dewpoint is NAN");
    return EXHAUST_FAN_ZONE_0;
  }

  if (dp <= this->get_stage1_clear_dp()) {
    return EXHAUST_FAN_ZONE_0;
  } else if (dp < this->get_stage1_trip_dp()) {
    return EXHAUST_FAN_ZONE_1;
  } else if (dp < this->get_stage2_clear_dp()) {
    return EXHAUST_FAN_ZONE_2;
  } else if (dp < this->get_stage2_trip_dp()) {
    return EXHAUST_FAN_ZONE_3;
  } else {
    return EXHAUST_FAN_ZONE_4;
  }
}

void ExhaustFan::compute_state_() {
  if (!this->controller_enabled_) {
    return;
  }

  ExhaustFanZone current_zone = this->get_current_zone_();
  if ((current_zone == this->previous_zone_) && !this->force_update_) {
    return;
  }

  switch (current_zone) {
    case EXHAUST_FAN_ZONE_0:
      ESP_LOGD(TAG, "Exhaust Fan Zone 0: Idle");
      this->idle_trigger_->trigger();
      break;
    case EXHAUST_FAN_ZONE_1:
      if ((this->previous_zone_ > EXHAUST_FAN_ZONE_2) ||
          (this->force_update_)) {
        ESP_LOGD(TAG, "Exhaust Fan Zone 1: Stage 1");
        this->stage1_trigger_->trigger();
      } else {
        ESP_LOGD(TAG, "Exhaust Fan Zone 1: No Change");
      }
      break;
    case EXHAUST_FAN_ZONE_2:
      ESP_LOGD(TAG, "Exhaust Fan Zone 2: Stage 1");
      this->stage1_trigger_->trigger();
      break;
    case EXHAUST_FAN_ZONE_3:
      if ((this->previous_zone_ < EXHAUST_FAN_ZONE_2) ||
          (this->force_update_)) {
        ESP_LOGD(TAG, "Exhaust Fan Zone 3: Stage 1");
        this->stage1_trigger_->trigger();
      } else {
        ESP_LOGD(TAG, "Exhaust Fan Zone 3: No Change");
      }
      break;
    case EXHAUST_FAN_ZONE_4:
      ESP_LOGD(TAG, "Exhaust Fan Zone 4: Stage 2");
      this->stage2_trigger_->trigger();
      break;
    default:
      ESP_LOGE(TAG, "Unknown Exhaust Fan Zone");
  }

  this->previous_zone_ = current_zone;
}

void ExhaustFan::update() {
  this->force_update_ = true;
  this->compute_state_();
  this->force_update_ = false;
}

Trigger<> *ExhaustFan::get_idle_trigger() { return this->idle_trigger_; }
Trigger<> *ExhaustFan::get_stage1_trigger() { return this->stage1_trigger_; }
Trigger<> *ExhaustFan::get_stage2_trigger() { return this->stage2_trigger_; }


} // namespace exhaust_fan
} // namespace esphome