#ifndef CMBML__COMMON__HPP_
#define CMBML__COMMON__HPP_

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
    no_data,
    timeout
  };

  // reflection would be cool
  const char * error_code_string(StatusCode code);

}  // namespace cmbml

#endif  // CMBML__CDR__HPP_
