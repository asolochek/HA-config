import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, i2c
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_NUM_LEDS,
    CONF_OUTPUT_ID,
)

CODEOWNERS = ["@asolochek"]
DEPENDENCIES = ["i2c"]

qwiic_led_stick_ns = cg.esphome_ns.namespace("qwiic_led_stick")

QwiicLEDStick = qwiic_led_stick_ns.class_(
    "QwiicLEDStick", light.AddressableLight, i2c.I2CDevice
)

CONFIG_SCHEMA = light.ADDRESSABLE_LIGHT_SCHEMA.extend(
      {
          cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(QwiicLEDStick),
          cv.Optional(CONF_NUM_LEDS, default=10): cv.positive_not_null_int,
  #            cv.Optional(CONF_MAX_REFRESH_RATE, default=60): cv.positive_int,
      } 
    ).extend(i2c.i2c_device_schema(default_address=0x23)
)
 

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], config[CONF_NUM_LEDS])
    await light.register_light(var, config)
    await i2c.register_i2c_device(var, config)
    await cg.register_component(var, config)

#    cg.add(var.set_num_leds(config[CONF_NUM_LEDS]))