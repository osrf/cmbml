#ifndef CMBML__CDR_COMMON__HPP_
#define CMBML__CDR_COMMON__HPP_

#include <cinttypes>
#include <vector>

namespace cmbml {
  // Encapsulation identifiers
  // TODO Read encapsulation identifier from packet and condition serialization scheme on that.
  enum struct EncapsulationId : uint16_t {
    cdr_be = 0x0,  // Big-endian
    cdr_le = 0x1,  // Little-endian
    pl_cdr_be = 0x2,  // Parameter-list big-endian
    pl_cdr_le = 0x3,  // Parameter-list little-endian
  };

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

#endif  // CMBML__CDR_COMMON__HPP_
