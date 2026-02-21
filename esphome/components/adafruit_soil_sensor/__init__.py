# import esphome.codegen as cg
# import esphome.config_validation as cv

# from esphome.const import (
#   CONF_ID
# )

# CODEOWNERS = ['@asolochek']

# MULTI_CONF = True

# adafruit_soil_sensor_ns = cg.esphome_ns.namespace('adafruit_soil_sensor')
# AdafruitSoilSensorComponent = adafruit_soil_sensor_ns.class_('AdafruitSoilSensorComponent', cg.Component)

# CONF_ADAFRUIT_SOIL_SENSOR_ID = 'adafruit_soil_sensor_id'

# CONFIG_SCHEMA = cv.All(
#   cv.Schema({
#     cv.GenerateID(): cv.declare_id(AdafruitSoilSensorComponent),
#   })
#   .extend(cv.COMPONENT_SCHEMA),
# )

# async def to_code(config):
#   var = cg.new_Pvariable(config[CONF_ID])
#   await cg.register_component(var, config)