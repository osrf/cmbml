#ifndef CMBML__PLACE_INTEGRAL_TYPE__HPP_
#define CMBML__PLACE_INTEGRAL_TYPE__HPP_

#include <climits>
#include <type_traits>

namespace cmbml {

template<typename T>
constexpr size_t number_of_bits() {
  return sizeof(T)*CHAR_BIT;
}

// place an integral type into a long
// i represents the bitwise placement index of src
// start at low index, MSBs
//
// For example, suppose we are placing the byte 0x42 into a uint32 at bit index 8
// that would result in a long with the following layout:
// byte 0          | byte 1        | byte 2       | byte 3
// 0...2...........7...............15.............23...............31
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 0 0 0 0 0 0 0 0 | 1 0 0 0 0 1 0 | ...
//
// Preserve the value of dst: only overwrite the bits we need to replace
// TODO Unit test me
template<typename IntegralType>
void place_integral_type(const IntegralType src, uint32_t & dst, size_t & i) {
  static_assert(
      std::is_integral<IntegralType>::value,
      "Received non-integral type in primitive serialization!");

  // We must be able to fit the src type into dst
  static_assert(
      sizeof(IntegralType) <= sizeof(uint32_t),
      "Couldn't fit serialized integral type into a uint32_t!");

  // We need to fit something of size src into the long at the specified index
  // We expect src to span from index i to index "i + sizeof(IntegralType)"
  assert(i < number_of_bits<uint32_t>() - number_of_bits<IntegralType>());

  uint32_t widened_src = src;
  // Bitshift src to the index which it will land at 
  widened_src = widened_src << i;

  // zero the part of dst which we plan to replace
  uint32_t mask = UINT32_MAX;
  mask = mask << i;  // produces zeros until the first index
  mask |= UINT32_MAX >> (i + number_of_bits<IntegralType>());

  dst &= mask;
  // OR dst with the widened src
  dst |= src;
  i = (i + number_of_bits<IntegralType>()) % number_of_bits<uint32_t>();
}

// Bool specialization for flags
// We may end up using uint8_t instead
template<>
void place_integral_type(const bool src, uint32_t & dst, size_t & i) {
  assert(i < number_of_bits<uint32_t>());
  uint32_t mask = 1;
  mask = mask << i++;

  if (src) {
    // OR with mask
      dst |= mask;
  } else {
    // Invert mask and AND
    mask = ~mask;
    dst &= mask;
  }
}

// Specialization for uint32_t
template<>
void place_integral_type(const uint32_t src, uint32_t & dst, size_t & i) {
  assert(i == 0);
  dst = src;
}

}

#endif  // CMBML__PLACE_INTEGRAL_TYPE__HPP_
