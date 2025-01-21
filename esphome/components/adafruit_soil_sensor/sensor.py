import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_CALIBRATION,
    CONF_CALIBRATE_LINEAR,
    CONF_MIN_VALUE,
    CONF_MAX_VALUE,
    CONF_OFFSET,
    CONF_GAIN,
    CONF_TEMPERATURE,
    CONF_MOISTURE,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_MOISTURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    UNIT_CELSIUS,
    ICON_THERMOMETER,
    ICON_WATER_PERCENT,
    ICON_WATER
)

DEPENDENCIES = ['i2c']
AUTO_LOAD = ['climate_utils']

adafruit_soil_sensor_ns = cg.esphome_ns.namespace('adafruit_soil_sensor')

AdafruitSoilSensorComponent = adafruit_soil_sensor_ns.class_('AdafruitSoilSensorComponent', cg.PollingComponent, i2c.I2CDevice)

TEMPERATURE_SENSOR = cv.Schema(
  {
    cv.Optional(CONF_OFFSET, default=0): cv.float_,
    cv.Optional(CONF_GAIN, default=1): cv.float_,
  }
)

MOISTURE_SENSOR = cv.Schema(
  {
    cv.Optional(CONF_MIN_VALUE, default=200): cv.int_range(0, 65535),
    cv.Optional(CONF_MAX_VALUE, default=1015): cv.int_range(0, 65535),
  }
)

CONFIG_SCHEMA = ( 
    cv.Schema(
      {
        cv.GenerateID(): cv.declare_id(AdafruitSoilSensorComponent),
        cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
          unit_of_measurement=UNIT_CELSIUS,
          icon=ICON_THERMOMETER, 
          accuracy_decimals=1,
          device_class=DEVICE_CLASS_TEMPERATURE, 
          state_class=STATE_CLASS_MEASUREMENT,
        ).extend(TEMPERATURE_SENSOR),
        cv.Optional(CONF_MOISTURE): sensor.sensor_schema(
          unit_of_measurement=UNIT_PERCENT,
          icon=ICON_WATER_PERCENT,
          accuracy_decimals=0,
          device_class=DEVICE_CLASS_MOISTURE, 
          state_class=STATE_CLASS_MEASUREMENT,
        ).extend(MOISTURE_SENSOR),
      }
    )
    .extend(i2c.i2c_device_schema(default_address=0x36))
    .extend(cv.polling_component_schema("60s"))
)

async def to_code(config):
  var = cg.new_Pvariable(config[CONF_ID])
  await cg.register_component(var, config)
  await i2c.register_i2c_device(var, config)
  
  if temperature_config := config.get(CONF_TEMPERATURE):
    sens = await sensor.new_sensor(temperature_config)
    cg.add(var.set_temperature_sensor(sens))
    cg.add(var.set_temperature_calibration(config[CONF_TEMPERATURE][CONF_GAIN], config[CONF_TEMPERATURE][CONF_OFFSET]))
   
  if moisture_config := config.get(CONF_MOISTURE):
    sens = await sensor.new_sensor(moisture_config)
    cg.add(var.set_moisture_sensor(sens))
    cg.add(var.set_moisture_calibration(config[CONF_MOISTURE][CONF_MIN_VALUE], config[CONF_MOISTURE][CONF_MAX_VALUE]))
    
