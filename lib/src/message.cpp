#include "message.hpp"

#include <atomic>

namespace fcwt {

#define MESSAGE_TYPE_TO_STRING_CASE(x) \
  case message_type::x:                \
    return #x

char const* to_string(message_type type) {
  switch (type) {
    MESSAGE_TYPE_TO_STRING_CASE(hello);
    MESSAGE_TYPE_TO_STRING_CASE(start);
    MESSAGE_TYPE_TO_STRING_CASE(stop);
    MESSAGE_TYPE_TO_STRING_CASE(image_info_by_index);
    MESSAGE_TYPE_TO_STRING_CASE(thumbnail_by_index);
    MESSAGE_TYPE_TO_STRING_CASE(shutter);
    MESSAGE_TYPE_TO_STRING_CASE(single_part);
    MESSAGE_TYPE_TO_STRING_CASE(two_part);
    MESSAGE_TYPE_TO_STRING_CASE(full_image);
    MESSAGE_TYPE_TO_STRING_CASE(focus_point);
    MESSAGE_TYPE_TO_STRING_CASE(shutter_speed);
    MESSAGE_TYPE_TO_STRING_CASE(aperture);
    MESSAGE_TYPE_TO_STRING_CASE(exposure_correction);
    MESSAGE_TYPE_TO_STRING_CASE(camera_remote);
    MESSAGE_TYPE_TO_STRING_CASE(camera_last_image);
    MESSAGE_TYPE_TO_STRING_CASE(camera_capabilities);
    default:
      return "";
  }
}

bool fuji_message(native_socket const sockfd, uint32_t const id, void const* message,
                  size_t size) {
  fuji_send(sockfd, message, size);

  uint8_t buffer[8];
  size_t receivedBytes = fuji_receive_log(sockfd, buffer);

  if (!is_success_response(id, buffer, receivedBytes)) {
    log(LOG_DEBUG, string_format("received %zd bytes ", receivedBytes).append(hex_format(buffer, receivedBytes)));
    return false;
  }

  return true;
}

bool is_success_response(uint32_t const id, void const* buffer,
                         size_t const size) {
  if (size != 8) return false;

  struct response_success {
    uint8_t const type[4] = {0x03, 0x00, 0x01, 0x20};
    uint32_t id;
  };

  response_success success = {};
  success.id = id;
  bool const result = memcmp(&success, buffer, 8) == 0;
  if (!result) {
    log(LOG_WARN, std::string("expected: ").append(hex_format(&success, 8)));
    log(LOG_WARN, std::string("actual: ").append(hex_format(buffer, 8)));
  }
  return result;
}

uint32_t generate_message_id() {
  static std::atomic<uint32_t> id_counter;
  return ++id_counter;
}

}  // namespace fcwt
