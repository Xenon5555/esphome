#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace paj7620u2 {

static const char *const TAG = "paj7620u2";

enum PAJ7620U2_GESTURE {
  GES_NONE = 0, /**< No gesture */
  GES_UP,       /**< Upwards gesture */
  GES_DOWN,     /**< Downward gesture */
  GES_LEFT,     /**< Leftward gesture */
  GES_RIGHT,    /**< Rightward gesture */
  GES_CW,       /**< Clockwise circular gesture */
  GES_CCW,      /**< Counter clockwise circular gesture */
  GES_WAVE      /**< Wave gesture */
};

// Bank select
static const uint8_t PAJ7620U2_BANK_0 = 0x00;  // some registers are located in Bank 0
static const uint8_t PAJ7620U2_BANK_1 = 0x01;  // some registers are located in Bank 1

// Gesture update rate
static const uint8_t PAJ7620U2_UPDATE_NORMAL = 0xAC;  // Gesture Update Rate is 120Hz, Gesture speed is 60°/s - 600°/s
static const uint8_t PAJ7620U2_UPDATE_FAST = 0x30;  // Gesture Update Rate is 240Hz, Gesture speed is 60°/s - 1200°/s

// Read PAJ7620_ADDR_GES_PS_DET_FLAG_0
static const uint8_t PAJ7620U2_GESTURE_NONE = 0x00;
static const uint8_t PAJ7620U2_GESTURE_UP = 0x01;
static const uint8_t PAJ7620U2_GESTURE_DOWN = 0x02;
static const uint8_t PAJ7620U2_GESTURE_LEFT = 0x04;
static const uint8_t PAJ7620U2_GESTURE_RIGHT = 0x08;
static const uint8_t PAJ7620U2_GESTURE_CW = 0x40;
static const uint8_t PAJ7620U2_GESTURE_CCW = 0x80;
// Read PAJ7620_ADDR_GES_PS_DET_FLAG_1
static const uint8_t PAJ7620U2_GESTURE_WAVE = 0x01;

// DEVICE ID
static const uint8_t PAJ7620_PARTID_LOW = 0x20;
static const uint8_t PAJ7620_PARTID_HIGH = 0x76;

// REGISTER BANK SELECT
static const uint8_t PAJ7620_REGISTER_BANK_SEL = 0xEF;

// REGISTER BANK 0
static const uint8_t PAJ7620_ADDR_PART_ID_LOW = 0x00;
static const uint8_t PAJ7620_ADDR_PART_ID_HIGH = 0x01;
static const uint8_t PAJ7620_ADDR_VERSION_ID = 0x02;
static const uint8_t PAJ7620_ADDR_SUSPEND_CMD = 0x03;
static const uint8_t PAJ7620_ADDR_GES_PS_DET_MASK_0 = 0x41;
static const uint8_t PAJ7620_ADDR_GES_PS_DET_MASK_1 = 0x42;
static const uint8_t PAJ7620_ADDR_GES_PS_DET_FLAG_0 = 0x43;
static const uint8_t PAJ7620_ADDR_GES_PS_DET_FLAG_1 = 0x44;
static const uint8_t PAJ7620_ADDR_STATE_INDICATOR = 0x45;
static const uint8_t PAJ7620_ADDR_PS_HIGH_THRESHOLD = 0x69;
static const uint8_t PAJ7620_ADDR_PS_LOW_THRESHOLD = 0x6A;
static const uint8_t PAJ7620_ADDR_PS_APPROACH_STATE = 0x6B;
static const uint8_t PAJ7620_ADDR_PS_RAW_DATA = 0x6C;

// REGISTER BANK 1
static const uint8_t PAJ7620_ADDR_PS_GAIN = 0x44;
static const uint8_t PAJ7620_ADDR_R_IDLE_TIME_0 = 0x65;  // Register for Update Rate (FPS)
static const uint8_t PAJ7620_ADDR_R_IDLE_TIME_1 = 0x66;
static const uint8_t PAJ7620_ADDR_IDLE_S1_STEP_0 = 0x67;
static const uint8_t PAJ7620_ADDR_IDLE_S1_STEP_1 = 0x68;
static const uint8_t PAJ7620_ADDR_IDLE_S2_STEP_0 = 0x69;
static const uint8_t PAJ7620_ADDR_IDLE_S2_STEP_1 = 0x6A;
static const uint8_t PAJ7620_ADDR_OP_TO_S1_STEP_0 = 0x6B;
static const uint8_t PAJ7620_ADDR_OP_TO_S1_STEP_1 = 0x6C;
static const uint8_t PAJ7620_ADDR_OP_TO_S2_STEP_0 = 0x6D;
static const uint8_t PAJ7620_ADDR_OP_TO_S2_STEP_1 = 0x6E;
static const uint8_t PAJ7620_ADDR_OPERATION_ENABLE = 0x72;

// PAJ7620_SUSPEND_CMD
static const uint8_t PAJ7620_I2C_WAKEUP = 0x01;
static const uint8_t PAJ7620_I2C_SUSPEND = 0x00;

// PAJ7620_OPERATION_ENABLE
static const uint8_t PAJ7620_ENABLE = 0x01;
static const uint8_t PAJ7620_DISABLE = 0x00;

/**
 * Initial device register addresses and values.
 * Puts device into gesture mode with various "normal" mode values.
 */
const unsigned short initRegisterArray[] = {
    0xEF00,  // Bank 0
    0x4100,  // Disable interrupts for first 8 gestures
    0x4200,  // Disable wave (and other modes') interrupt(s)
    0x3707, 0x3817, 0x3906, 0x4201, 0x462D, 0x470F, 0x483C, 0x4900, 0x4A1E, 0x4C22,
    0x5110, 0x5E10, 0x6027, 0x8042, 0x8144, 0x8204, 0x8B01, 0x9006, 0x950A, 0x960C,
    0x9705, 0x9A14, 0x9C3F, 0xA519, 0xCC19, 0xCD0B, 0xCE13, 0xCF64, 0xD021,
    0xEF01,  // Bank 1
    0x020F, 0x0310, 0x0402, 0x2501, 0x2739, 0x287F, 0x2908, 0x3EFF, 0x5E3D,
    0x6596,  // R_IDLE_TIME LSB - Set sensor speed to 'normal speed' - 120 fps
    0x6797, 0x69CD, 0x6A01, 0x6D2C, 0x6E01, 0x7201, 0x7335,
    0x7400,  // Set to gesture mode
    0x7701,
    0xEF00,  // Bank 0
    0x41FF,  // Re-enable interrupts for first 8 gestures
    0x4201   // Re-enable interrupts for wave gesture
};

static const unsigned long INIT_REG_ARRAY_SIZE = sizeof(initRegisterArray) / sizeof(initRegisterArray[0]);

class PAJ7620U2Component : public PollingComponent, public i2c::I2CDevice {
  SUB_BINARY_SENSOR(up)
  SUB_BINARY_SENSOR(down)
  SUB_BINARY_SENSOR(left)
  SUB_BINARY_SENSOR(right)
  SUB_BINARY_SENSOR(cw)
  SUB_BINARY_SENSOR(ccw)
  SUB_BINARY_SENSOR(wave)

 public:
  void setup() override;
  void dump_config() override;
  void update() override;

  PAJ7620U2_GESTURE readGesture();
  void publish_states(PAJ7620U2_GESTURE gesture);

 protected:
  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
    WRONG_CHIP_ID,
  } error_code_{NONE};

  bool selectBank(uint8_t bank);
  void writeRegisterArray(const unsigned short array[], int arraySize);
};

}  // namespace paj7620u2
}  // namespace esphome
