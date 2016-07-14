#ifndef CMBML__CDR_COMMON__HPP_
#define CMBML__CDR_COMMON__HPP_

#include <cinttypes>
#include <vector>

namespace cmbml {

// TODO Allocator
template<typename Allocator = std::allocator<uint32_t>>
using Packet = std::vector<uint32_t, Allocator>;

enum struct StatusCode {
  ok,
  precondition_violated,
  postcondition_violated,
  not_yet_implemented,  // Reached a feature that was not implemented
  packet_invalid,
  out_of_memory,  // specifically, packet did not have enough memory allocated
  deserialize_failed,
  no_data
};

// reflection would be cool
const char * error_code_string(StatusCode code) {
  switch(code) {
    case StatusCode::ok:
      return "StatusCode::ok";
    case StatusCode::precondition_violated:
      return "StatusCode::precondition_violated";
    case StatusCode::postcondition_violated:
      return "StatusCode::postcondition_violated";
    case StatusCode::not_yet_implemented:
      return "StatusCode::not_yet_implemented";
    case StatusCode::packet_invalid:
      return "StatusCode::packet_invalid";
    case StatusCode::out_of_memory:
      return "StatusCode::out_of_memory";
    case StatusCode::deserialize_failed:
      return "StatusCode::deserialize_failed";
    case StatusCode::no_data:
      return "StatusCode::no_data";
    default:
      return "ERROR: invalid status code reached";
  }
  return "";
}

}  // namespace cmbml

#endif  // CMBML__CDR_COMMON__HPP_
