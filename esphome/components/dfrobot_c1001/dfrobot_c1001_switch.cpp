#include "dfrobot_c1001_switch.h"

namespace esphome {
namespace dfrobot_c1001 {

static const char *const TAG = "dfrobot_c1001_switch";

void C1001HPLedSwitch::write_state(bool state) {
  if (parent_->get_init_state()) {
    this->parent_->enqueue_command(0x01, 0x03, {static_cast<uint8_t>(state)}, 5);  // HP Led Set command
    this->parent_->enqueue_command(0x01, 0x02, {0x0F}, 6);                         // Module Reset command
  } else {
    ESP_LOGW(TAG, "Can not write switch state. Sensor not initialized!");
  }
}

void C1001FallLedSwitch::write_state(bool state) {
  if (parent_->get_init_state()) {
    this->parent_->enqueue_command(0x01, 0x04, {static_cast<uint8_t>(state)}, 5);  // Fall Led Set command
    this->parent_->enqueue_command(0x01, 0x02, {0x0F}, 6);                         // Module Reset command
  } else {
    ESP_LOGW(TAG, "Can not write switch state. Sensor not initialized!");
  }
}

}  // namespace dfrobot_c1001
}  // namespace esphome
