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
  not_yet_implemented,
  packet_invalid,
  out_of_memory
};

}  // namespace cmbml

#endif  // CMBML__CDR_COMMON__HPP_
