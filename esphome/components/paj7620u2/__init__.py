import esphome.codegen as cg
from esphome.components import binary_sensor, i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["binary_sensor"]
MULTI_CONF = True

paj7620u2_ns = cg.esphome_ns.namespace("paj7620u2")
PAJ7620U2Component = paj7620u2_ns.class_(
    "PAJ7620U2Component", cg.PollingComponent, i2c.I2CDevice
)

CONF_GESTURE_UP = "up"
CONF_GESTURE_DOWN = "down"
CONF_GESTURE_LEFT = "left"
CONF_GESTURE_RIGHT = "right"
CONF_GESTURE_CW = "cw"
CONF_GESTURE_CCW = "ccw"
CONF_GESTURE_WAVE = "wave"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(CONF_GESTURE_UP): binary_sensor.binary_sensor_schema(
                icon="mdi:arrow-up"
            ),
            cv.Optional(CONF_GESTURE_DOWN): binary_sensor.binary_sensor_schema(
                icon="mdi:arrow-down"
            ),
            cv.Optional(CONF_GESTURE_LEFT): binary_sensor.binary_sensor_schema(
                icon="mdi:arrow-left"
            ),
            cv.Optional(CONF_GESTURE_RIGHT): binary_sensor.binary_sensor_schema(
                icon="mdi:arrow-right"
            ),
            cv.Optional(CONF_GESTURE_CW): binary_sensor.binary_sensor_schema(
                icon="mdi:reload"
            ),
            cv.Optional(CONF_GESTURE_CCW): binary_sensor.binary_sensor_schema(
                icon="mdi:restore"
            ),
            cv.Optional(CONF_GESTURE_WAVE): binary_sensor.binary_sensor_schema(
                icon="mdi:restore"
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(default_address=0x73))
    .extend({cv.GenerateID(): cv.declare_id(PAJ7620U2Component)})
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_GESTURE_UP in config:
        up_binary_sensor = await binary_sensor.new_binary_sensor(
            config[CONF_GESTURE_UP]
        )
        cg.add(var.set_up_binary_sensor(up_binary_sensor))

    if CONF_GESTURE_DOWN in config:
        down_binary_sensor = await binary_sensor.new_binary_sensor(
            config[CONF_GESTURE_DOWN]
        )
        cg.add(var.set_down_binary_sensor(down_binary_sensor))

    if CONF_GESTURE_LEFT in config:
        left_binary_sensor = await binary_sensor.new_binary_sensor(
            config[CONF_GESTURE_LEFT]
        )
        cg.add(var.set_left_binary_sensor(left_binary_sensor))

    if CONF_GESTURE_RIGHT in config:
        right_binary_sensor = await binary_sensor.new_binary_sensor(
            config[CONF_GESTURE_RIGHT]
        )
        cg.add(var.set_right_binary_sensor(right_binary_sensor))

    if CONF_GESTURE_CW in config:
        cw_binary_sensor = await binary_sensor.new_binary_sensor(
            config[CONF_GESTURE_CW]
        )
        cg.add(var.set_cw_binary_sensor(cw_binary_sensor))

    if CONF_GESTURE_CCW in config:
        ccw_binary_sensor = await binary_sensor.new_binary_sensor(
            config[CONF_GESTURE_CCW]
        )
        cg.add(var.set_ccw_binary_sensor(ccw_binary_sensor))

    if CONF_GESTURE_WAVE in config:
        wave_binary_sensor = await binary_sensor.new_binary_sensor(
            config[CONF_GESTURE_WAVE]
        )
        cg.add(var.set_wave_binary_sensor(wave_binary_sensor))
