#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif

namespace esphome {
namespace dfrobot_c1001 {

class DFRobotC1001;

struct QueuedCommand {
  uint8_t ctrl_word;
  uint8_t cmd_word;
  std::vector<uint8_t> cmd_data;
  int priority;  // Add priority to the command
  // For std::priority_queue to work:
  bool operator>(const QueuedCommand &other) const { return priority > other.priority; }
};

class DFRobotC1001 : public Component, public uart::UARTDevice {
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(hardware_version)
  SUB_TEXT_SENSOR(firmware_version)
#endif
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(human_presence)
#endif
#ifdef USE_SENSOR
  SUB_SENSOR(motion_information)
  SUB_SENSOR(human_position_x)
  SUB_SENSOR(human_position_y)
  SUB_SENSOR(human_position_z)
  SUB_SENSOR(bed_entry_status)
  SUB_SENSOR(sleep_state)
  SUB_SENSOR(heart_rate)
  SUB_SENSOR(respiration_rate)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(hp_led)
  SUB_SWITCH(fall_led)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(module_restart)
#endif

 public:
  void dump_config() override;
  void loop() override;

  void process_uart_data();
  void process_commands();
  void enqueue_command(uint8_t ctrl_word, uint8_t cmd_word, std::vector<uint8_t> cmd_data, int priority);
  bool get_init_state() { return init_done_; }

  void set_operation_mode(uint8_t operation_mode) { this->operation_mode_ = operation_mode; }
  void set_ignore_unknown_reply(bool ignore_unknown_reply) { this->ignore_unknown_reply_ = ignore_unknown_reply; }

 protected:
  static constexpr size_t UART_RX_BUFFER_SIZE = 128;  // Buffer size
  std::array<uint8_t, UART_RX_BUFFER_SIZE> uart_rx_buffer_;
  size_t uart_rx_length_ = 0;

  bool ignore_unknown_reply_{true};
  uint8_t operation_mode_{1};
  uint8_t request_step_ = 0;
  bool init_done_ = false;
  uint8_t act_operation_mode_ = 0;
  uint32_t last_valid_response_time_ = 0;
  static constexpr uint32_t COMMUNICATION_TIMEOUT = 70000;  // Timeout after 70 seconds
  bool sensor_initialized_ = false;                         // Tracks whether initialization is complete

  // Command tracking
  QueuedCommand pending_command_;
  bool awaiting_response_ = false;
  uint32_t last_request_time_ = 0;
  uint32_t last_info_query_time_ = 0;   // Last info query time
  uint32_t last_state_query_time_ = 0;  // Last state query time
  uint8_t retry_count_ = 0;
  static constexpr uint32_t COMMAND_TIMEOUT = 8000;        // Timeout after 8 seconds
  static constexpr uint8_t MAX_RETRIES = 3;                // Skip command after 3 retries
  static constexpr uint32_t INFO_QUERY_INTERVAL = 60000;   // 60 sec info query interval
  static constexpr uint32_t STATE_QUERY_INTERVAL = 10000;  // 10 sec state query interval

 private:
  // Priority queue
  std::priority_queue<QueuedCommand, std::vector<QueuedCommand>, std::greater<QueuedCommand>> command_queue;

  bool send_command(uint8_t ctrl_word, uint8_t cmd_word, std::vector<uint8_t> cmd_data);
  uint8_t calculate_checksum(const std::vector<uint8_t> &data, std::vector<uint8_t>::const_iterator start,
                             std::vector<uint8_t>::const_iterator end);
  int16_t parse_custom_signed_int16(const std::vector<uint8_t> &data, size_t index);
  std::string convert_to_hex_string(const std::vector<uint8_t> &vector);
  void log_unknown_command(uint8_t ctrl_word, uint8_t cmd_word);
};

}  // namespace dfrobot_c1001
}  // namespace esphome
