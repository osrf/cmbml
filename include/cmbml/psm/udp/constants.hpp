#ifndef CMBML__PSM__UDP__CONSTANTS_HPP_
#define CMBML__PSM__UDP__CONSTANTS_HPP_

#include <cmbml/types.hpp>

namespace cmbml {
namespace udp {
  enum UserEntity : uint8_t {
    unknown = 0x00,
    writer_with_key = 0x02
    writer_no_key = 0x03
    reader_no_key = 0x04
    reader_with_key = 0x07
  }

  enum BuiltinEntity : uint8_t {
    unknown = 0xc0,
    participant = 0xc1,
    writer_with_key = 0xc2
    writer_no_key = 0xc3
    reader_no_key = 0xc4
    reader_with_key = 0xc7
  }

  // This is truly amazing
  // Think about your life and your choiches
  // May need to make this mapping available in udp/Context
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER 0x00000001 << 0;
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR 0x00000001 << 1;
  #define DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER 0x00000001 << 2;
  #define DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR 0x00000001 << 3;
  #define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER 0x00000001 << 4;
  #define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR 0x00000001 << 5;
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER 0x00000001 << 6;
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR 0x00000001 << 7;
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER 0x00000001 << 8;
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR 0x00000001 << 9;
  #define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER 0x00000001 << 10;
  #define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER 0x00000001 << 11;

  #define LOCATOR_KIND_INVALID -1
  #define LOCATOR_PORT_INVALID 0
  #define LOCATOR_KIND_RESERVED 0
  #define LOCATOR_KIND_UDPv4 1
  #define LOCATOR_KIND_UDPv6 2


}  // namespace udp
}  // namespace cmbml

#endif  // CMBML__PSM__UDP__CONSTANTS_HPP_
