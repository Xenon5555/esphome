#include "dfrobot_c1001_button.h"

namespace esphome {
namespace dfrobot_c1001 {

static const char *const TAG = "dfrobot_c1001_button";

void C1001ModuleRestartButton::press_action() {
  this->parent_->enqueue_command(0x01, 0x02, {0x0F}, 5);  // Module Reset command
  ESP_LOGI(TAG, "Module Reset Button pressed.");
}

}  // namespace dfrobot_c1001
}  // namespace esphome
