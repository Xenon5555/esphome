import esphome.codegen as cg
from esphome.components import binary_sensor, button, sensor, switch, text_sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_OCCUPANCY,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["text_sensor", "binary_sensor", "sensor", "switch", "button"]
MULTI_CONF = True

CONF_C1001_ID = "dfrobot_c1001_id"

# Define a namespace for the component
dfrobot_c1001_ns = cg.esphome_ns.namespace("dfrobot_c1001")
DFRobotC1001 = dfrobot_c1001_ns.class_("DFRobotC1001", cg.Component, uart.UARTDevice)

# Switch class
C1001HPLedSwitch = dfrobot_c1001_ns.class_("C1001HPLedSwitch", switch.Switch)
C1001FallLedSwitch = dfrobot_c1001_ns.class_("C1001FallLedSwitch", switch.Switch)

# Button class
C1001ModuleRestartButton = dfrobot_c1001_ns.class_(
    "C1001ModuleRestartButton", button.Button
)

# Define base
CONF_OPERATION_MODE = "operation_mode"
CONF_IGNORE_UNKNOWN_REPLY = "ignore_unknown_reply"

# Define categories
CONF_CAT_SYSTEM_FUNCTION = "system_function"
CONF_CAT_HUMAN_EXISTENCE = "human_existence"
CONF_CAT_FALL_DETECTION = "fall_detection"
CONF_CAT_SLEEP_MONITORING = "sleep_monitoring"

# Define sensor configurations
CONF_HARDWARE_VERSION = "hardware_version"
CONF_FIRMWARE_VERSION = "firmware_version"
CONF_HUMAN_PRESENCE_STATE = "human_presence"
CONF_MOTION_INFORMATION = "motion_information"
CONF_HUMAN_POSITION_X = "human_position_x"
CONF_HUMAN_POSITION_Y = "human_position_y"
CONF_HUMAN_POSITION_Z = "human_position_z"
CONF_BED_ENTRY_STATUS = "bed_entry_status"
CONF_SLEEP_STATE = "sleep_state"
CONF_HEART_RATE = "heart_rate"
CONF_RESPIRATION_RATE = "respiration_rate"

# Define switch configurations
CONF_HP_LED = "hp_led"
CONF_FALL_LED = "fall_led"

# Define button configurations
CONF_MODULE_RESTART = "module_restart"


def validate_mode_dependent_options(config):
    operation_mode = config.get(CONF_OPERATION_MODE)

    if operation_mode == 1:
        if CONF_CAT_SLEEP_MONITORING in config:
            raise cv.Invalid(
                f"{CONF_CAT_SLEEP_MONITORING} is not allowed in operation_mode 1"
            )

        if CONF_CAT_HUMAN_EXISTENCE in config:
            if CONF_HUMAN_POSITION_X in config[CONF_CAT_HUMAN_EXISTENCE]:
                raise cv.Invalid(
                    f"{CONF_HUMAN_POSITION_X} is not allowed in operation_mode 1"
                )
            if CONF_HUMAN_POSITION_Y in config[CONF_CAT_HUMAN_EXISTENCE]:
                raise cv.Invalid(
                    f"{CONF_HUMAN_POSITION_Y} is not allowed in operation_mode 1"
                )
            if CONF_HUMAN_POSITION_Z in config[CONF_CAT_HUMAN_EXISTENCE]:
                raise cv.Invalid(
                    f"{CONF_HUMAN_POSITION_Z} is not allowed in operation_mode 1"
                )

    elif operation_mode == 2:
        if CONF_CAT_FALL_DETECTION in config:
            raise cv.Invalid(
                f"{CONF_CAT_FALL_DETECTION} is not allowed in operation_mode 2"
            )

        if CONF_CAT_SYSTEM_FUNCTION in config:
            if CONF_FALL_LED in config[CONF_CAT_SYSTEM_FUNCTION]:
                raise cv.Invalid(f"{CONF_FALL_LED} is not allowed in operation_mode 2")

    elif operation_mode is None:
        pass
    else:
        raise cv.Invalid(f"Invalid operation_mode: {operation_mode}")

    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DFRobotC1001),
            cv.Required(CONF_OPERATION_MODE): cv.int_range(min=1, max=2),
            cv.Optional(CONF_IGNORE_UNKNOWN_REPLY, default=True): cv.boolean,
            cv.Optional(CONF_CAT_SYSTEM_FUNCTION): cv.Schema(
                {
                    cv.Optional(CONF_HARDWARE_VERSION): text_sensor.text_sensor_schema(
                        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                        icon="mdi:identifier",
                    ),
                    cv.Optional(CONF_FIRMWARE_VERSION): text_sensor.text_sensor_schema(
                        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                        icon="mdi:identifier",
                    ),
                    cv.Optional(CONF_HP_LED): switch.switch_schema(
                        class_=C1001HPLedSwitch,
                        entity_category=ENTITY_CATEGORY_CONFIG,
                        icon="mdi:led-outline",
                        default_restore_mode="RESTORE_DEFAULT_OFF",
                    ),
                    cv.Optional(CONF_FALL_LED): switch.switch_schema(
                        class_=C1001FallLedSwitch,
                        entity_category=ENTITY_CATEGORY_CONFIG,
                        icon="mdi:led-outline",
                        default_restore_mode="RESTORE_DEFAULT_OFF",
                    ),
                    cv.Optional(CONF_MODULE_RESTART): button.button_schema(
                        class_=C1001ModuleRestartButton,
                        entity_category=ENTITY_CATEGORY_CONFIG,
                        icon="mdi:restart",
                    ),
                }
            ),
            cv.Optional(CONF_CAT_HUMAN_EXISTENCE): cv.Schema(
                {
                    cv.Optional(
                        CONF_HUMAN_PRESENCE_STATE
                    ): binary_sensor.binary_sensor_schema(
                        device_class=DEVICE_CLASS_OCCUPANCY
                    ),
                    cv.Optional(CONF_MOTION_INFORMATION): sensor.sensor_schema(),
                    cv.Optional(CONF_HUMAN_POSITION_X): sensor.sensor_schema(
                        state_class=STATE_CLASS_MEASUREMENT,
                        icon="mdi:axis-x-arrow",
                        unit_of_measurement="cm",
                        accuracy_decimals=0,
                    ),
                    cv.Optional(CONF_HUMAN_POSITION_Y): sensor.sensor_schema(
                        state_class=STATE_CLASS_MEASUREMENT,
                        icon="mdi:axis-y-arrow",
                        unit_of_measurement="cm",
                        accuracy_decimals=0,
                    ),
                    cv.Optional(CONF_HUMAN_POSITION_Z): sensor.sensor_schema(
                        state_class=STATE_CLASS_MEASUREMENT,
                        icon="mdi:axis-z-arrow",
                        unit_of_measurement="cm",
                        accuracy_decimals=0,
                    ),
                }
            ),
            cv.Optional(CONF_CAT_SLEEP_MONITORING): cv.Schema(
                {
                    cv.Optional(CONF_BED_ENTRY_STATUS): sensor.sensor_schema(),
                    cv.Optional(CONF_SLEEP_STATE): sensor.sensor_schema(
                        icon="mdi:sleep"
                    ),
                    cv.Optional(CONF_HEART_RATE): sensor.sensor_schema(
                        state_class=STATE_CLASS_MEASUREMENT,
                        icon="mdi:heart",
                        unit_of_measurement="bpm",
                        accuracy_decimals=0,
                    ),
                    cv.Optional(CONF_RESPIRATION_RATE): sensor.sensor_schema(
                        state_class=STATE_CLASS_MEASUREMENT,
                        icon="mdi:lungs",
                        unit_of_measurement="bpm",
                        accuracy_decimals=0,
                    ),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA),
    validate_mode_dependent_options,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    # Register the component
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_operation_mode(config[CONF_OPERATION_MODE]))
    if CONF_IGNORE_UNKNOWN_REPLY in config:
        cg.add(var.set_ignore_unknown_reply(config[CONF_IGNORE_UNKNOWN_REPLY]))

    # Create sensors
    if CONF_CAT_SYSTEM_FUNCTION in config:  # Check if the category exists
        system_function_config = config[CONF_CAT_SYSTEM_FUNCTION]

        if CONF_HARDWARE_VERSION in system_function_config:
            hardware_sensor = await text_sensor.new_text_sensor(
                system_function_config[CONF_HARDWARE_VERSION]
            )
            cg.add(var.set_hardware_version_text_sensor(hardware_sensor))
        if CONF_FIRMWARE_VERSION in system_function_config:
            firmware_sensor = await text_sensor.new_text_sensor(
                system_function_config[CONF_FIRMWARE_VERSION]
            )
            cg.add(var.set_firmware_version_text_sensor(firmware_sensor))
        if CONF_HP_LED in system_function_config:
            hp_led_switch = await switch.new_switch(system_function_config[CONF_HP_LED])
            await cg.register_parented(hp_led_switch, config[CONF_ID])
            cg.add(var.set_hp_led_switch(hp_led_switch))
        if CONF_FALL_LED in system_function_config:
            fall_led_switch = await switch.new_switch(
                system_function_config[CONF_FALL_LED]
            )
            await cg.register_parented(fall_led_switch, config[CONF_ID])
            cg.add(var.set_fall_led_switch(fall_led_switch))
        if CONF_MODULE_RESTART in system_function_config:
            module_restart_button = await button.new_button(
                system_function_config[CONF_MODULE_RESTART]
            )
            await cg.register_parented(module_restart_button, config[CONF_ID])
            cg.add(var.set_module_restart_button(module_restart_button))

    if CONF_CAT_HUMAN_EXISTENCE in config:  # Check if the category exists
        human_existence_config = config[CONF_CAT_HUMAN_EXISTENCE]

        if CONF_HUMAN_PRESENCE_STATE in human_existence_config:
            human_presence_sensor = await binary_sensor.new_binary_sensor(
                human_existence_config[CONF_HUMAN_PRESENCE_STATE]
            )
            cg.add(var.set_human_presence_binary_sensor(human_presence_sensor))
        if CONF_MOTION_INFORMATION in human_existence_config:
            motion_information_sensor = await sensor.new_sensor(
                human_existence_config[CONF_MOTION_INFORMATION]
            )
            cg.add(var.set_motion_information_sensor(motion_information_sensor))
        if CONF_HUMAN_POSITION_X in human_existence_config:
            human_position_x_sensor = await sensor.new_sensor(
                human_existence_config[CONF_HUMAN_POSITION_X]
            )
            cg.add(var.set_human_position_x_sensor(human_position_x_sensor))
        if CONF_HUMAN_POSITION_Y in human_existence_config:
            human_position_y_sensor = await sensor.new_sensor(
                human_existence_config[CONF_HUMAN_POSITION_Y]
            )
            cg.add(var.set_human_position_y_sensor(human_position_y_sensor))
        if CONF_HUMAN_POSITION_Z in human_existence_config:
            human_position_z_sensor = await sensor.new_sensor(
                human_existence_config[CONF_HUMAN_POSITION_Z]
            )
            cg.add(var.set_human_position_z_sensor(human_position_z_sensor))

    if CONF_CAT_SLEEP_MONITORING in config:  # Check if the category exists
        sleep_monitoring_config = config[CONF_CAT_SLEEP_MONITORING]

        if CONF_BED_ENTRY_STATUS in sleep_monitoring_config:
            bed_entry_status_sensor = await sensor.new_sensor(
                sleep_monitoring_config[CONF_BED_ENTRY_STATUS]
            )
            cg.add(var.set_bed_entry_status_sensor(bed_entry_status_sensor))
        if CONF_SLEEP_STATE in sleep_monitoring_config:
            sleep_state_sensor = await sensor.new_sensor(
                sleep_monitoring_config[CONF_SLEEP_STATE]
            )
            cg.add(var.set_sleep_state_sensor(sleep_state_sensor))
        if CONF_HEART_RATE in sleep_monitoring_config:
            heart_rate_sensor = await sensor.new_sensor(
                sleep_monitoring_config[CONF_HEART_RATE]
            )
            cg.add(var.set_heart_rate_sensor(heart_rate_sensor))
        if CONF_RESPIRATION_RATE in sleep_monitoring_config:
            respiration_rate_sensor = await sensor.new_sensor(
                sleep_monitoring_config[CONF_RESPIRATION_RATE]
            )
            cg.add(var.set_respiration_rate_sensor(respiration_rate_sensor))
