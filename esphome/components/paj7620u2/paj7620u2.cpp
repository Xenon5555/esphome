#include "paj7620u2.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace paj7620u2 {

void PAJ7620U2Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PAJ7620U2...");
  uint8_t chip_id_low = 0, chip_id_high = 0;

  // Mark as not failed before initializing. Some devices will turn off sensors to save on batteries
  // and when they come back on, COMPONENT FAILED must be unset on the component.
  if (this->is_failed()) {
    this->reset_to_construction_state();
  }

  /* There's two register banks (0 & 1) to be selected between.
   * BANK0 is where most data collection operations happen, so it's default.
   * Selecting the bank is done here twice for a reason. When the 7620 turns
   * on, the I2C bus is sleeping. When you first read/write to the bus
   * the 7620 wakes up, but it sometimes misses that first message.
   */
  selectBank(PAJ7620U2_BANK_0);  // This is done twice on purpose
  selectBank(PAJ7620U2_BANK_0);  // Default operations on BANK0

  if (this->read_register(PAJ7620_ADDR_PART_ID_LOW, &chip_id_low, 1) ||
      this->read_register(PAJ7620_ADDR_PART_ID_HIGH, &chip_id_high, 1)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  if (chip_id_low != PAJ7620_PARTID_LOW || chip_id_high != PAJ7620_PARTID_HIGH) {
    this->error_code_ = WRONG_CHIP_ID;
    this->mark_failed();
    return;
  }

  // Initialize registers
  writeRegisterArray(initRegisterArray, INIT_REG_ARRAY_SIZE);
}

void PAJ7620U2Component::dump_config() {
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "PAJ7620U2:");
  switch (this->error_code_) {
    case COMMUNICATION_FAILED:
      ESP_LOGE(TAG, "Communication with PAJ7620U2 failed!");
      break;
    case WRONG_CHIP_ID:
      ESP_LOGE(TAG, "PAJ7620U2 has wrong chip ID!");
      break;
    case NONE:
    default:
      break;
  }
  LOG_UPDATE_INTERVAL(this);

#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "Gesture UP", this->up_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Gesture DOWN", this->down_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Gesture LEFT", this->left_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Gesture RIGHT", this->right_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Gesture CW", this->cw_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Gesture CCW", this->ccw_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Gesture WAVE", this->wave_binary_sensor_);
#endif
}

void PAJ7620U2Component::update() { publish_states(readGesture()); }

PAJ7620U2_GESTURE PAJ7620U2Component::readGesture() {
  uint8_t data = 0, data1 = 0, readCode = 0;
  PAJ7620U2_GESTURE result = GES_NONE;

  selectBank(PAJ7620U2_BANK_0);
  readCode = this->read_register(PAJ7620_ADDR_GES_PS_DET_FLAG_0, &data, 1);
  if (readCode) {  // Read error
    ESP_LOGW(TAG, "Error on gesture read: %d", readCode);
    return GES_NONE;
  } else {
    switch (data) {
      case PAJ7620U2_GESTURE_UP:
        result = GES_UP;
        break;

      case PAJ7620U2_GESTURE_DOWN:
        result = GES_DOWN;
        break;

      case PAJ7620U2_GESTURE_LEFT:
        result = GES_LEFT;
        break;

      case PAJ7620U2_GESTURE_RIGHT:
        result = GES_RIGHT;
        break;

      case PAJ7620U2_GESTURE_CW:
        result = GES_CW;
        break;

      case PAJ7620U2_GESTURE_CCW:
        result = GES_CCW;
        break;

      default:
        readCode = this->read_register(PAJ7620_ADDR_GES_PS_DET_FLAG_1, &data1, 1);  // Bank 1 (Reg 0x44) has wave flag
        if (readCode) {                                                             // Read error
          ESP_LOGW(TAG, "Error on gesture read: %d", readCode);
          return GES_NONE;
        } else if (data1 == PAJ7620U2_GESTURE_WAVE) {
          result = GES_WAVE;
        }
        break;
    }
  }
  ESP_LOGD(TAG, "Gesture read result: %d", result);
  return result;
}

void PAJ7620U2Component::publish_states(PAJ7620U2_GESTURE gesture) {
  if (this->up_binary_sensor_ != nullptr) {
    gesture == GES_UP ? up_binary_sensor_->publish_state(true) : up_binary_sensor_->publish_state(false);
  }
  if (this->down_binary_sensor_ != nullptr) {
    gesture == GES_DOWN ? down_binary_sensor_->publish_state(true) : down_binary_sensor_->publish_state(false);
  }
  if (this->left_binary_sensor_ != nullptr) {
    gesture == GES_LEFT ? left_binary_sensor_->publish_state(true) : left_binary_sensor_->publish_state(false);
  }
  if (this->right_binary_sensor_ != nullptr) {
    gesture == GES_RIGHT ? right_binary_sensor_->publish_state(true) : right_binary_sensor_->publish_state(false);
  }
  if (this->cw_binary_sensor_ != nullptr) {
    gesture == GES_CW ? cw_binary_sensor_->publish_state(true) : cw_binary_sensor_->publish_state(false);
  }
  if (this->ccw_binary_sensor_ != nullptr) {
    gesture == GES_CCW ? ccw_binary_sensor_->publish_state(true) : ccw_binary_sensor_->publish_state(false);
  }
  if (this->wave_binary_sensor_ != nullptr) {
    gesture == GES_WAVE ? wave_binary_sensor_->publish_state(true) : wave_binary_sensor_->publish_state(false);
  }
}

bool PAJ7620U2Component::selectBank(uint8_t bank) { return this->write_register(PAJ7620_REGISTER_BANK_SEL, &bank, 1); }

void PAJ7620U2Component::writeRegisterArray(const unsigned short array[], int arraySize) {
  for (unsigned int i = 0; i < arraySize; i++) {
    uint16_t word = array[i];
    uint8_t address, value;

    address = (word & 0xFF00) >> 8;
    value = (word & 0x00FF);
    this->write_register(address, &value, 1);
  }
  selectBank(PAJ7620U2_BANK_0);  // Guarantee parking in BANK0
}

}  // namespace paj7620u2
}  // namespace esphome
