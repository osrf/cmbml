#ifndef CMBML__CDR_COMMON__HPP_
#define CMBML__CDR_COMMON__HPP_

#include <cmbml/cdr/place_integral_type.hpp>

// TODO Allocator
template<typename Allocator = std::allocator<uint32_t>>
using Packet = std::vector<uint32_t, Allocator>;



#endif  // CMBML__CDR_COMMON__HPP_
