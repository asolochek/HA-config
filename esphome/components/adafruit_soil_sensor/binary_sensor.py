import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, sensor
from esphome.const import (
    CONF_SENSOR_ID,
    CONF_THRESHOLD,
    CONF_HYSTERESIS,
)

adafruit_soil_sensor_ns = cg.esphome_ns.namespace('adafruit_soil_sensor')
AdafruitSoilSensorComponent = adafruit_soil_sensor_ns.class_('AdafruitSoilBinarySensorComponent', binary_sensor.BinarySensor, cg.Component)

CONFIG_SCHEMA = (
  binary_sensor.binary_sensor_schema(AdafruitSoilSensorComponent)
  .extend(
    {
      cv.Required(CONF_SENSOR_ID): cv.use_id(sensor.Sensor),
      cv.Required(CONF_THRESHOLD): cv.int_range(0, 100),
      cv.Optional(CONF_HYSTERESIS, default=10): cv.int_range(0, 100),
    }
  )
  .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
  var = await binary_sensor.new_binary_sensor(config)
  await cg.register_component(var, config)
  
  sens = await cg.get_variable(config[CONF_SENSOR_ID])
  cg.add(var.set_sensor(sens))
  
  cg.add(var.set_threshold(config[CONF_THRESHOLD]))
  cg.add(var.set_hysteresis(config[CONF_HYSTERESIS]))