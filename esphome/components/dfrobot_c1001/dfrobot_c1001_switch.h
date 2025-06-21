#pragma once

#include "esphome/components/switch/switch.h"
#include "dfrobot_c1001.h"

namespace esphome {
namespace dfrobot_c1001 {

class C1001HPLedSwitch : public switch_::Switch, public Parented<DFRobotC1001> {
 public:
  C1001HPLedSwitch() = default;

 protected:
  void write_state(bool state) override;
};

class C1001FallLedSwitch : public switch_::Switch, public Parented<DFRobotC1001> {
 public:
  C1001FallLedSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace dfrobot_c1001
}  // namespace esphome
