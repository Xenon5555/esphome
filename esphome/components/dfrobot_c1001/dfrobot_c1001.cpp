#include "dfrobot_c1001.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace dfrobot_c1001 {

static const char *const TAG = "dfrobot_c1001";

void DFRobotC1001::dump_config() {
  ESP_LOGCONFIG(TAG, "DFRobot_C1001:");
  this->check_uart_settings(115200);
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR("  ", "Hardware Version", this->hardware_version_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Firmware Version", this->firmware_version_text_sensor_);
#endif
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "Human Presence", this->human_presence_binary_sensor_);
#endif
#ifdef USE_SENSOR
  LOG_SENSOR("  ", "Motion Information Status", this->motion_information_sensor_);
  LOG_SENSOR("  ", "Human Position X", this->human_position_x_sensor_);
  LOG_SENSOR("  ", "Human Position Y", this->human_position_y_sensor_);
  LOG_SENSOR("  ", "Human Position Z", this->human_position_z_sensor_);
  LOG_SENSOR("  ", "Bed Entry Status", this->bed_entry_status_sensor_);
  LOG_SENSOR("  ", "Sleep State Status", this->sleep_state_sensor_);
  LOG_SENSOR("  ", "Heart Rate", this->heart_rate_sensor_);
  LOG_SENSOR("  ", "Respiration Rate", this->respiration_rate_sensor_);
#endif
#ifdef USE_SWITCH
  LOG_SWITCH("  ", "Human Presence LED", this->hp_led_switch_);
  LOG_SWITCH("  ", "Fall Detection LED", this->fall_led_switch_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON("  ", "Module Restart", this->module_restart_button_);
#endif
}

void DFRobotC1001::loop() {
  uint32_t now = millis();

  // Read UART buffer
  while (available()) {
    uint8_t byte = read();
    size_t next_byte_pos = (uart_rx_length_ + 1) % UART_RX_BUFFER_SIZE;
    if (next_byte_pos != 0) {  // Prevent buffer overflow
      uart_rx_buffer_[uart_rx_length_] = byte;
      uart_rx_length_ = next_byte_pos;
    } else {
      uart_rx_length_ = 0;
      ESP_LOGE(TAG, "UART buffer overflow, dropping data");
      break;
    }
  }
  // Check length
  if (uart_rx_length_ >= 10) {
    // Check for frame header and tail (0x53 0x59 and 0x54 0x43)
    if (uart_rx_buffer_[0] == 0x53 && uart_rx_buffer_[1] == 0x59 && uart_rx_buffer_[uart_rx_length_ - 2] == 0x54 &&
        uart_rx_buffer_[uart_rx_length_ - 1] == 0x43) {
      process_uart_data();  // Process data
      uart_rx_length_ = 0;  // "Clear" buffer after processing
      last_valid_response_time_ = now;
    }
  }

  // Detect lost communication
  if (sensor_initialized_ && (now - last_valid_response_time_ > COMMUNICATION_TIMEOUT)) {
    ESP_LOGW(TAG, "Communication timeout! Reinitializing sensor...");
    sensor_initialized_ = false;
    request_step_ = 0;  // Restart initialization process
  }

  // Send commands
  process_commands();
}

void DFRobotC1001::process_uart_data() {
  std::vector<uint8_t> buffer(uart_rx_buffer_.begin(), uart_rx_buffer_.begin() + uart_rx_length_);

  if (calculate_checksum(buffer, buffer.begin(), buffer.end() - 3) != buffer.at(buffer.size() - 3)) {
    ESP_LOGW(TAG, "Checksum error");
    return;  // Checksum error
  }

  // Extract data from frame
  uint8_t control_word = buffer[2];
  uint8_t command_word = buffer[3];
  uint16_t data_length = (static_cast<uint16_t>(buffer[4]) << 8) | buffer[5];  // Big-endian
  std::vector<uint8_t> uart_data(buffer.begin() + 6, buffer.end() - 3);

#ifdef ESPHOME_LOG_HAS_DEBUG
  ESP_LOGD(TAG, "Received: control: 0x%02X, command: 0x%02X, length: %i, data (hex): %s", control_word, command_word,
           data_length, convert_to_hex_string(uart_data).c_str());
#endif

  // Check if response matches pending command
  if (awaiting_response_ && control_word == pending_command_.ctrl_word && command_word == pending_command_.cmd_word) {
    awaiting_response_ = false;  // Clear pending command
    ESP_LOGD(TAG, "Matched response for control: 0x%02X, command: 0x%02X", pending_command_.ctrl_word,
             pending_command_.cmd_word);
  }

  // Process response
  switch (control_word) {
    // ================================================================================
    // System function
    // ================================================================================
    case 0x01:
      switch (command_word) {
        case 0x01:  // Heartbeat
        case 0x80:
          ESP_LOGI(TAG, "Received Heartbeat");
          break;

        case 0x02:  // Module Reset
          ESP_LOGI(TAG, "Received Module Reset...");
          break;

        case 0x03:  // HP Led Switch command
          if (this->hp_led_switch_ != nullptr) {
            hp_led_switch_->publish_state(uart_data.front() == 0x01);
            ESP_LOGI(TAG, "Set HP Led Switch State: %s", hp_led_switch_->state ? "True" : "False");
          }
        case 0x83:  // HP Led Switch querry
          if (this->hp_led_switch_ != nullptr) {
            if (hp_led_switch_->state != static_cast<bool>(uart_data.front())) {
              enqueue_command(0x01, 0x03, {static_cast<uint8_t>(hp_led_switch_->state)}, 2);
              ESP_LOGD(TAG, "Received HP Led Switch state does not match setting. Sending state...");
            };
          }
          break;

        case 0x04:  // Fall Led Switch command
          if (this->fall_led_switch_ != nullptr) {
            fall_led_switch_->publish_state(uart_data.front() == 0x01);
            ESP_LOGI(TAG, "Set Fall Led Switch State: %s", fall_led_switch_->state ? "True" : "False");
          }
          break;
        case 0x84:  // Fall Led Switch querry
          if (this->fall_led_switch_ != nullptr && operation_mode_ == 1) {
            if (fall_led_switch_->state != static_cast<bool>(uart_data.front())) {
              enqueue_command(0x01, 0x04, {static_cast<uint8_t>(fall_led_switch_->state)}, 2);
              ESP_LOGD(TAG, "Received Fall Led Switch state does not match setting. Sending state...");
            };
          }
          break;

        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
      // Product information/settings
      // ================================================================================
    case 0x02:
      switch (command_word) {
        case 0x03:  // Hardware Model
        case 0xA3:
          if (this->hardware_version_text_sensor_ != nullptr) {
            std::string data_str(uart_data.begin(), uart_data.end());
            hardware_version_text_sensor_->publish_state(data_str);
            ESP_LOGI(TAG, "Received Hardware Version: %s", data_str.c_str());
          }
          break;

        case 0x04:  // Firmware Version
        case 0xA4:
          if (this->firmware_version_text_sensor_ != nullptr) {
            std::string data_str(uart_data.begin(), uart_data.end());
            firmware_version_text_sensor_->publish_state(data_str);
            ESP_LOGI(TAG, "Received Firmware Version: %s", data_str.c_str());
          }
          break;

        case 0x08:  // Work Mode
        case 0xA8:
          act_operation_mode_ = uart_data.front();
          ESP_LOGI(TAG, "Received Work Mode: %i", uart_data.front());
          break;

        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
      // Working status
      // ================================================================================
    case 0x05:
      switch (command_word) {
        case 0x01:  // Initialization Completion Information
        case 0x81:
          init_done_ = uart_data.front() == 0x01 || uart_data.front() == 0x00 ||
                       uart_data.front() == 0x0F;  // 0x00 / 0x0F = Assume init is done
          ESP_LOGI(TAG, "Received Initialization Completion Information: %i", uart_data.front());
          break;

        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
      // Radar Installation information
      // ================================================================================
    case 0x06:
      switch (command_word) {
        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
      // Human existence
      // ================================================================================
    case 0x80:
      switch (command_word) {
        case 0x01:  // Human Presence Status
        case 0x81:
          if (this->human_presence_binary_sensor_ != nullptr) {
            human_presence_binary_sensor_->publish_state(uart_data.front() != 0);
            ESP_LOGI(TAG, "Received Human Presence Status: %s", uart_data.front() ? "True" : "False");
          }
          break;

        case 0x02:  // Motion Information
        case 0x82:
          if (this->motion_information_sensor_ != nullptr) {
            motion_information_sensor_->publish_state(uart_data.front());
            ESP_LOGI(TAG, "Received Motion Information: %i", uart_data.front());
          }
          break;

        case 0x05:  // Human Position
        case 0x85:
          if (this->human_position_x_sensor_ != nullptr) {
            int16_t x = parse_custom_signed_int16(uart_data, 0);
            human_position_x_sensor_->publish_state(x);
            ESP_LOGI(TAG, "Human Position X value: %i", x);
          }
          if (this->human_position_y_sensor_ != nullptr) {
            int16_t y = parse_custom_signed_int16(uart_data, 2);
            human_position_y_sensor_->publish_state(y);
            ESP_LOGI(TAG, "Human Position Y value: %i", y);
          }
          if (this->human_position_z_sensor_ != nullptr) {
            int16_t z = parse_custom_signed_int16(uart_data, 4);
            human_position_z_sensor_->publish_state(z);
            ESP_LOGI(TAG, "Human Position Z value: %i", z);
          }
          break;

        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
      // Respiration detection
      // ================================================================================
    case 0x81:
      switch (command_word) {
        case 0x02:  // Respiration Rate
        case 0x82:
          if (this->respiration_rate_sensor_ != nullptr) {
            respiration_rate_sensor_->publish_state(static_cast<uint8_t>(uart_data.front()));
            ESP_LOGI(TAG, "Received Respiration Rate: %d", uart_data.front());
          }
          break;

        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
      // Sleep monitoring
      // ================================================================================
    case 0x84:
      switch (command_word) {
        case 0x01:  // Bed Entry/Exit Status
        case 0x81:
          if (this->bed_entry_status_sensor_ != nullptr) {
            bed_entry_status_sensor_->publish_state(uart_data.front());
            ESP_LOGI(TAG, "Received Bed Entry/Exit Status: %i", uart_data.front());
          }
          break;

        case 0x02:  // Sleep Status
        case 0x82:
          if (this->sleep_state_sensor_ != nullptr) {
            sleep_state_sensor_->publish_state(uart_data.front());
            ESP_LOGI(TAG, "Received Sleep State Status: %i", uart_data.front());
          }
          break;

        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
      // Heart rate monitoring
      // ================================================================================
    case 0x85:
      switch (command_word) {
        case 0x02:  // Heart Rate
        case 0x82:
          if (this->heart_rate_sensor_ != nullptr) {
            heart_rate_sensor_->publish_state(static_cast<uint8_t>(uart_data.front()));
            ESP_LOGI(TAG, "Received Heart Rate: %d", uart_data.front());
          }
          break;

        default:
          log_unknown_command(control_word, command_word);
      }
      break;

      // ================================================================================
    default:
      if (!ignore_unknown_reply_)
        ESP_LOGW(TAG, "Received unknown control word: 0x%02X", control_word);
  }
}

void DFRobotC1001::process_commands() {
  uint32_t now = millis();

  // If a command is pending, check for timeout
  if (awaiting_response_ && now - last_request_time_ > COMMAND_TIMEOUT) {
    retry_count_++;
    if (retry_count_ > MAX_RETRIES) {
      ESP_LOGE(TAG, "Command failed after %d retries: control: 0x%02X, command: 0x%02X. Reinitializing...", MAX_RETRIES,
               pending_command_.ctrl_word, pending_command_.cmd_word);
      awaiting_response_ = false;  // Drop this command
      retry_count_ = 0;
      request_step_ = 0;  // Restart initialization
    } else {
      ESP_LOGW(TAG, "Retrying command (%d/%d)...", retry_count_, MAX_RETRIES);
      send_command(pending_command_.ctrl_word, pending_command_.cmd_word, pending_command_.cmd_data);
      last_request_time_ = now;
    }
    return;
  }

  if (awaiting_response_) {
    return;  // Skip further processing
  } else if (!awaiting_response_ &&
             !command_queue.empty()) {  // If there's no pending command, fetch the next one from the queue
    pending_command_ = command_queue.top();
    command_queue.pop();
    if (send_command(pending_command_.ctrl_word, pending_command_.cmd_word, pending_command_.cmd_data)) {
      awaiting_response_ = true;
      last_request_time_ = now;
      retry_count_ = 0;  // Reset retry counter
      return;            // Skip further processing
    }
  }

  // State machine
  switch (request_step_) {
    case 0:  // Start initialization
      ESP_LOGI(TAG, "Initializing DFRobot C1001 Sensor...");
      init_done_ = false;
      sensor_initialized_ = false;
      act_operation_mode_ = 0;
      enqueue_command(0x05, 0x81, {0x0F}, 0);  // Query initialization status
      request_step_ = 1;
      break;

    case 1:  // Wait for initialization completion
      if (init_done_) {
        enqueue_command(0x02, 0xA8, {0x0F}, 1);  // Query operation mode
        request_step_ = 2;
      } else if (!awaiting_response_) {
        ESP_LOGW(TAG, "Initialization check failed. Retrying...");
        request_step_ = 0;
      }
      break;

    case 2:  // Validate operation mode
      if (act_operation_mode_ != 0 && act_operation_mode_ == operation_mode_) {
        ESP_LOGI(TAG, "Basic initialization complete. Running in mode: %d", operation_mode_);
        request_step_ = 3;  // Mode OK, send parameters
      } else if (act_operation_mode_ != 0) {
        ESP_LOGI(TAG, "Incorrect operation mode (%d). Updating...", act_operation_mode_);
        enqueue_command(0x02, 0x08, {operation_mode_}, 1);
        request_step_ = 0;  // Re-check states
      }
      break;

    case 3:  // Send parameters
      ESP_LOGI(TAG, "Sending parameters then restarting sensor");

      // HP Led Set command
      if (this->hp_led_switch_ != nullptr) {
        enqueue_command(0x01, 0x03, {static_cast<uint8_t>(hp_led_switch_->state)}, 2);
      }
      // Fall Led Set command
      if (this->fall_led_switch_ != nullptr && operation_mode_ == 1) {  // Send only on "Fall" mode
        enqueue_command(0x01, 0x04, {static_cast<uint8_t>(fall_led_switch_->state)}, 2);
      }

      enqueue_command(0x01, 0x02, {0x0F}, 3);  // Restart module
      request_step_ = 4;
      break;

    case 4:  // Wait for transmission done
      if (!awaiting_response_) {
        sensor_initialized_ = true;
        request_step_ = 10;  // Init done
      }
      break;

    // ================================================================================
    case 10:
      // Queue state information querry
      if (now - last_state_query_time_ > STATE_QUERY_INTERVAL) {
        if (this->human_presence_binary_sensor_ != nullptr) {
          enqueue_command(0x80, 0x81, {0x0F}, 9);  // Existence Information
        }
        if (this->motion_information_sensor_ != nullptr) {
          enqueue_command(0x80, 0x82, {0x0F}, 9);  // Motion Information
        }
        if (this->human_position_x_sensor_ != nullptr || this->human_position_y_sensor_ != nullptr ||
            this->human_position_z_sensor_ != nullptr) {
          enqueue_command(0x80, 0x85, {0x0F}, 9);  // Human Position
        }
        if (this->bed_entry_status_sensor_ != nullptr) {
          enqueue_command(0x84, 0x81, {0x0F}, 9);  // Bed Entry Status
        }
        if (this->sleep_state_sensor_ != nullptr) {
          enqueue_command(0x84, 0x82, {0x0F}, 9);  // Sleep Status
        }
        if (this->heart_rate_sensor_ != nullptr) {
          enqueue_command(0x85, 0x82, {0x0F}, 9);  // Heart Rate
        }
        if (this->respiration_rate_sensor_ != nullptr) {
          enqueue_command(0x81, 0x82, {0x0F}, 9);  // Respiration Rate
        }
        last_state_query_time_ = now;
        ESP_LOGD(TAG, "Queued state querry commands");
      }

      // Queue base information querry
      if (now - last_info_query_time_ > INFO_QUERY_INTERVAL) {
        if (this->hardware_version_text_sensor_ != nullptr) {
          enqueue_command(0x02, 0xA3, {0x0F}, 10);  // Get Hardware Model
        }
        if (this->firmware_version_text_sensor_ != nullptr) {
          enqueue_command(0x02, 0xA4, {0x0F}, 10);  // Get Firmware Version
        }
        if (this->hp_led_switch_ != nullptr) {  // Get HP Led state
          enqueue_command(0x01, 0x83, {0x0F}, 10);
        }
        if (this->fall_led_switch_ != nullptr && operation_mode_ == 1) {  // Get Fall Led state
          enqueue_command(0x01, 0x84, {0x0F}, 10);
        }
        last_info_query_time_ = now;
        ESP_LOGD(TAG, "Queued information querry commands");
      }
      break;

    default:
      request_step_ = 0;
      break;
  }
}

uint8_t DFRobotC1001::calculate_checksum(const std::vector<uint8_t> &data, std::vector<uint8_t>::const_iterator start,
                                         std::vector<uint8_t>::const_iterator end) {
  if (start < data.begin() || end > data.end() || start > end) {
    ESP_LOGE(TAG, "Checksum calculation failed - Invalid start or end indices!");
  }
  uint16_t sum = 0;  // Use a 16-bit integer to avoid overflow during summation
  for (auto it = start; it != end; ++it) {
    sum += *it;  // Dereference the iterator to get the value
  }
  return static_cast<uint8_t>(sum % 256);  // Modulo 256 and cast back to uint8_t
}

std::string DFRobotC1001::convert_to_hex_string(const std::vector<uint8_t> &vector) {
  std::string hex_string;
  hex_string.reserve(vector.size() * 3);  // Reserve space to avoid reallocations

  char buffer[4];  // Temporary buffer for each hex value (including space and null terminator)

  for (uint8_t byte : vector) {
    std::snprintf(buffer, sizeof(buffer), "%02X ", byte);  // Format to hex with leading zero and space
    hex_string += buffer;
  }

  // Remove the trailing space
  if (!hex_string.empty()) {
    hex_string.pop_back();
  }

  return hex_string;
}

void DFRobotC1001::enqueue_command(uint8_t ctrl_word, uint8_t cmd_word, std::vector<uint8_t> cmd_data, int priority) {
  // Check if the command is already in queue
  std::priority_queue<QueuedCommand, std::vector<QueuedCommand>, std::greater<QueuedCommand>> temp_queue =
      command_queue;  // Create a copy
  while (!temp_queue.empty()) {
    const QueuedCommand &queued_cmd = temp_queue.top();
    if (queued_cmd.ctrl_word == ctrl_word && queued_cmd.cmd_word == cmd_word) {
      ESP_LOGW(TAG, "Duplicate command skipped: control: 0x%02X, command: 0x%02X", ctrl_word, cmd_word);
      return;
    }
    temp_queue.pop();
  }
  command_queue.push({ctrl_word, cmd_word, cmd_data, priority});
#ifdef ESPHOME_LOG_HAS_DEBUG
  ESP_LOGD(TAG, "Enqueued command: control: 0x%02X, command: 0x%02X, data: %s", ctrl_word, cmd_word,
           convert_to_hex_string(cmd_data).c_str());
#endif
}

bool DFRobotC1001::send_command(uint8_t ctrl_word, uint8_t cmd_word, std::vector<uint8_t> cmd_data) {
  // Build command
  std::vector<uint8_t> command;
  // Start bytes
  command.push_back(0x53);
  command.push_back(0x59);
  // Control and command words
  command.push_back(ctrl_word);
  command.push_back(cmd_word);
  // Data length
  uint16_t data_length = static_cast<uint16_t>(cmd_data.size());
  command.push_back(static_cast<uint8_t>((data_length >> 8) & 0xFF));  // High byte
  command.push_back(static_cast<uint8_t>(data_length & 0xFF));         // Low byte
  // Command data
  command.insert(command.end(), cmd_data.begin(), cmd_data.end());
  // Checksum
  command.push_back(calculate_checksum(command, command.begin(), command.end()));
  // End bytes
  command.push_back(0x54);
  command.push_back(0x43);

  // Send command
  write_array(command);
#ifdef ESPHOME_LOG_HAS_DEBUG
  ESP_LOGD(TAG, "Sent: control: 0x%02X, command: 0x%02X, data (hex): %s", ctrl_word, cmd_word,
           convert_to_hex_string(cmd_data).c_str());
#endif
  return true;
}

int16_t DFRobotC1001::parse_custom_signed_int16(const std::vector<uint8_t> &data, size_t index) {
  uint16_t raw_value = (data[index] << 8) | data[index + 1];

  ESP_LOGD(TAG, "Got raw position value %i from index %i (%X %X)", raw_value, index, data[index], data[index + 1]);

  if (raw_value & 0x8000) {        // If first bit is 1 (positive) convert to negative value
    return -(raw_value & 0x7FFF);  // Mask out the sign bit
  } else {
    return raw_value & 0x7FFF;
  }
}

void DFRobotC1001::log_unknown_command(uint8_t ctrl_word, uint8_t cmd_word) {
  if (!ignore_unknown_reply_)
    ESP_LOGW(TAG, "Received unknown command word: control: 0x%02X, command: 0x%02X", ctrl_word, cmd_word);
}

}  // namespace dfrobot_c1001
}  // namespace esphome
