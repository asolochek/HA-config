import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
)

CODEOWNERS = ["@asolochek"]
AUTO_LOAD = ["climate_utils"]

CONF_IDLE_ACTION = "idle_action"
CONF_STAGE1_ACTION = "stage1_action"
CONF_STAGE2_ACTION = "stage2_action"
CONF_TEMPERATURE_SENSOR = "temperature_sensor"
CONF_HUMIDITY_SENSOR = "humidity_sensor"
CONF_EXTERNAL_HUMIDITY_SENSOR = "external_humidity_sensor"
CONF_EXTERNAL_TEMPERATURE_SENSOR = "external_temperature_sensor"
CONF_MIN_HUMIDITY = "min_humidity"
CONF_STAGE1_TRIP = "stage1_trip"
CONF_STAGE2_TRIP = "stage2_trip"
CONF_BASELINE_OFFSET = "baseline_offset"
CONF_HYSTERESIS = "hysteresis"

exhaust_fan_ns = cg.esphome_ns.namespace("exhaust_fan")
ExhaustFan = exhaust_fan_ns.class_("ExhaustFan", cg.Component)


def validate_trip_values(config):
    if config[CONF_STAGE1_TRIP] >= config[CONF_STAGE2_TRIP]:
        raise cv.Invalid("Stage 1 trip must be less than Stage 2 trip")
    return config

def validate(config):
  if CONF_EXTERNAL_HUMIDITY_SENSOR in config:
    if CONF_EXTERNAL_TEMPERATURE_SENSOR not in config:
      raise cv.Invalid("External humidity sensor requires external_temperature_sensor")
  if CONF_EXTERNAL_TEMPERATURE_SENSOR in config:
    if CONF_EXTERNAL_HUMIDITY_SENSOR not in config:
      raise cv.Invalid("External temperature sensor requires external_humidity_sensor")
  if CONF_BASELINE_OFFSET in config:
    if CONF_EXTERNAL_HUMIDITY_SENSOR not in config:
      raise cv.Invalid("Baseline offset requires external_humidity_sensor and external_temperature_sensor")
  return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
      {
        cv.GenerateID(): cv.declare_id(ExhaustFan),
        cv.Required(CONF_TEMPERATURE_SENSOR): cv.use_id(sensor.Sensor),
        cv.Required(CONF_HUMIDITY_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_EXTERNAL_HUMIDITY_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_EXTERNAL_TEMPERATURE_SENSOR): cv.use_id(sensor.Sensor),
        cv.Required(CONF_IDLE_ACTION): automation.validate_automation(single=True),
        cv.Required(CONF_STAGE1_ACTION): automation.validate_automation(single=True),
        cv.Required(CONF_STAGE2_ACTION): automation.validate_automation(single=True),
        cv.Optional(CONF_BASELINE_OFFSET): cv.float_,
        cv.Optional(CONF_STAGE1_TRIP, 80): cv.float_,
        cv.Optional(CONF_STAGE2_TRIP, 90): cv.float_,
        cv.Optional(CONF_HYSTERESIS, 4): cv.float_,
        cv.Optional(CONF_MIN_HUMIDITY): cv.float_,
      }
    ),
    validate_trip_values,
    validate,
)


async def to_code(config):
  var = cg.new_Pvariable(config[CONF_ID])
  await cg.register_component(var, config)

  sens = await cg.get_variable(config[CONF_TEMPERATURE_SENSOR])
  cg.add(var.set_temperature_sensor(sens))

  sens = await cg.get_variable(config[CONF_HUMIDITY_SENSOR])
  cg.add(var.set_humidity_sensor(sens))
  
  if CONF_EXTERNAL_HUMIDITY_SENSOR in config:
      sens = await cg.get_variable(config[CONF_EXTERNAL_HUMIDITY_SENSOR])
      cg.add(var.set_external_humidity_sensor(sens))
      
  if CONF_EXTERNAL_TEMPERATURE_SENSOR in config:
      sens = await cg.get_variable(config[CONF_EXTERNAL_TEMPERATURE_SENSOR])
      cg.add(var.set_external_temperature_sensor(sens))
      
  await automation.build_automation(var.get_idle_trigger(), [], config[CONF_IDLE_ACTION])
  await automation.build_automation(var.get_stage1_trigger(), [], config[CONF_STAGE1_ACTION])
  await automation.build_automation(var.get_stage2_trigger(), [], config[CONF_STAGE2_ACTION])
  
  if CONF_BASELINE_OFFSET in config:
      cg.add(var.set_baseline_offset(config[CONF_BASELINE_OFFSET]))
  
  if CONF_STAGE1_TRIP in config:
      cg.add(var.set_stage1_trip(config[CONF_STAGE1_TRIP]))
      
  if CONF_STAGE2_TRIP in config:  
      cg.add(var.set_stage2_trip(config[CONF_STAGE2_TRIP]))
      
  if CONF_HYSTERESIS in config:    
      cg.add(var.set_hysteresis(config[CONF_HYSTERESIS]))
      
  if CONF_MIN_HUMIDITY in config:
      cg.add(var.set_min_humidity(config[CONF_MIN_HUMIDITY]))

    