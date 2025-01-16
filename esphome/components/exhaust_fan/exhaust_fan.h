#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"
#include "esphome/components/climate_utils/climate_utils.h"


namespace esphome {
namespace exhaust_fan {

/// Enum for different humidity ranges to determine fan speed
enum ExhaustFanZone {
  EXHAUST_FAN_ZONE_0 = 0,
  EXHAUST_FAN_ZONE_1 = 1,
  EXHAUST_FAN_ZONE_2 = 2,
  EXHAUST_FAN_ZONE_3 = 3,
  EXHAUST_FAN_ZONE_4 = 4
};

enum Units {
  CELSIUS,
  FAHRENHEIT
};



#define DBG(...) if(debug_) __VA_ARGS__

class ExhaustFan : public Component {
public:
  ExhaustFan();
  void setup() override;
  void dump_config() override;

  void set_temperature_sensor(sensor::Sensor *temperature_sensor);
  void set_humidity_sensor(sensor::Sensor *humidity_sensor);
  void set_external_temperature_sensor(sensor::Sensor *external_temperature_sensor);
  void set_external_humidity_sensor(sensor::Sensor *external_humidity_sensor);
  void set_stage1_trip(float stage1_trip);
  void set_stage2_trip(float stage2_trip);
  void set_stage1_trip_dp(float stage1_trip_dp);
  void set_stage2_trip_dp(float stage2_trip_dp);
  void set_stage1_clear_dp(float stage1_clear_dp);
  void set_stage2_clear_dp(float stage2_clear_dp);
  void set_baseline_offset(float baseline_offset);
  void set_hysteresis(float hysteresis);
  void set_min_humidity(float min_rh);
  void enable_controller(void);
  void disable_controller(void);
  void update(void);
  void set_debug(bool debug) { debug_ = debug; }

  float get_stage1_trip();
  float get_stage2_trip();
  float get_stage1_clear();
  float get_stage2_clear();
  float get_stage1_trip_dp();
  float get_stage2_trip_dp();
  float get_stage1_clear_dp();
  float get_stage2_clear_dp();
  float get_temperature();
  float get_humidity();
  float get_dewpoint();
  float get_external_temperature();
  float get_external_humidity();
  float get_external_dewpoint();
  float get_baseline_offset();
  float get_hysteresis();
  float get_min_humidity();
  bool get_controller_enabled() { return controller_enabled_; }
  bool get_debug() { return debug_; }

  Trigger<> *get_idle_trigger();
  Trigger<> *get_stage1_trigger();
  Trigger<> *get_stage2_trigger();

protected:
  void compute_state_();
  ExhaustFanZone get_current_zone_();
  ExhaustFanZone previous_zone_{EXHAUST_FAN_ZONE_1};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};
  sensor::Sensor *external_temperature_sensor_{nullptr};
  sensor::Sensor *external_humidity_sensor_{nullptr};

  bool force_update_{false};
  bool debug_{false};

  float stage1_trip_;
  float stage2_trip_;
  optional<float> stage1_clear_dp_;
  optional<float> stage1_trip_dp_;
  optional<float> stage2_clear_dp_;
  optional<float> stage2_trip_dp_;
  float hysteresis_{4.0};
  bool controller_enabled_{true};
  optional<float> baseline_offset_;
  optional<float> min_rh_;

  Trigger<> *idle_trigger_;
  Trigger<> *stage1_trigger_;
  Trigger<> *stage2_trigger_;
};


} // namespace exhaust_fan
}  // namespace esphome