import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import (
    CONF_FREQUENCY,
    CONF_ID,
    CONF_ADDRESS,
    CONF_I2C_ID,
    CONF_I2C,
)

DEPENDENCIES = ['i2c']

seesaw_ns = cg.esphome_ns.namespace('Adafruit_seesaw')
