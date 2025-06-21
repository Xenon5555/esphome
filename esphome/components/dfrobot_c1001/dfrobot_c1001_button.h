#pragma once

#include "esphome/components/button/button.h"
#include "dfrobot_c1001.h"

namespace esphome {
namespace dfrobot_c1001 {

class C1001ModuleRestartButton : public button::Button, public Parented<DFRobotC1001> {
 public:
  C1001ModuleRestartButton() = default;

 protected:
  void press_action() override;
};

}  // namespace dfrobot_c1001
}  // namespace esphome
