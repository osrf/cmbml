#ifndef CMBML__PSM__UDP__CONSTANTS_HPP_
#define CMBML__PSM__UDP__CONSTANTS_HPP_

#include <cmbml/types.hpp>

namespace cmbml {
namespace udp {

  // This is truly amazing
  // TODO Think about your life and your choiches
  // May need to make this mapping available in udp/Context
  // Consider also define bitwise operators on the enum
  // these names are awful
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER 0x00000001 << 0;  // builtin_spdp_writer
  #define DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR 0x00000001 << 1;  // builtin_spdp_reader
  #define DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER 0x00000001 << 2;  // builtin_sedp_pub_writer
  #define DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR 0x00000001 << 3;  // builtin_sedp_pub_reader
  #define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER 0x00000001 << 4;  // builtin_sedp_sub_writer
  #define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR 0x00000001 << 5;  // builtin_sedp_sub_reader
  // unsupported??? Hmm.
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
