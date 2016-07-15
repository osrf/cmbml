#ifndef CMBML__PLACE_INTEGRAL_TYPE__HPP_
#define CMBML__PLACE_INTEGRAL_TYPE__HPP_

#include <climits>
#include <type_traits>

#include <boost/hana/at_key.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/type.hpp>
/*
#include <boost/hana.hpp>
*/

namespace hana = boost::hana;


// I suspect these functions don't work for signed values
namespace cmbml {

// I couldn't find a compile-time association like this in the standard library.
constexpr auto max_value_map = hana::make_map(
  hana::make_pair(hana::type_c<char>,  CHAR_MAX),
  hana::make_pair(hana::type_c<uint8_t>,  UINT8_MAX),
  hana::make_pair(hana::type_c<uint16_t>, UINT16_MAX),
  hana::make_pair(hana::type_c<uint32_t>, UINT32_MAX),
  hana::make_pair(hana::type_c<uint64_t>, UINT64_MAX),
  hana::make_pair(hana::type_c<int8_t>,  INT8_MAX),
  hana::make_pair(hana::type_c<int16_t>, INT16_MAX),
  hana::make_pair(hana::type_c<int32_t>, INT32_MAX),
  hana::make_pair(hana::type_c<int64_t>, INT64_MAX)
);


// deceptively, this actually works for bool types
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
// TODO template specialization for "narrow", or make different f'ns for "widen" and "narrow"
template<
  typename SrcT,
  typename DstT,
  typename std::enable_if<std::is_integral<SrcT>::value &&
      std::is_integral<DstT>::value &&
      !std::is_same<SrcT, bool>::value && !std::is_same<DstT, bool>::value
      >::type * = nullptr>
void place_integral_type(const SrcT src, DstT & dst, const size_t i) {
  if (sizeof(SrcT) <= sizeof(DstT)) {
    // We need to fit something of size src into the long at the specified index
    // We expect src to span from index i to index "i + sizeof(IntegralType)"
    assert(i <= number_of_bits<DstT>() - number_of_bits<SrcT>());

    DstT widened_src = src;
    // Bitshift src to the index which it will land at 
    widened_src = widened_src << i;

    // zero the part of dst which we plan to replace: i to (i + bits(SrcT))
    // dst may have critical 

    DstT mask = max_value_map[hana::type_c<DstT>] << (i + number_of_bits<SrcT>());
    mask |= max_value_map[hana::type_c<DstT>] >> (number_of_bits<DstT>() - i);
    dst &= mask;

    // OR dst with the widened src
    dst |= widened_src;
  } else {  // Narrow
    // TODO
    dst = 0;
    assert(i <= number_of_bits<SrcT>() - number_of_bits<DstT>());
    // i refers to the index of src from which we will extract the bits to place into dst
    // Make a copy of src, bitshift, then mask
    SrcT copy = src;
    copy = copy >> i;
    SrcT mask = max_value_map[hana::type_c<SrcT>];
    mask = mask >> (number_of_bits<SrcT>() - number_of_bits<DstT>());

    copy &= mask;
    dst = copy;
    // index is incremented outside of this scope
  }
}


// In CDR bool must be aligned to a byte boundary
// (header flags are packed into bitsets, which resolve to a different specialization)
template<typename DstT>
void place_integral_type(const bool src, DstT & dst, const size_t i) {
  uint8_t tmp = src;
  place_integral_type(tmp, dst, i);
}

// In CDR bool must be aligned to a byte boundary
template<typename SrcT>
void place_integral_type(const SrcT src, bool & dst, const size_t i) {
  uint8_t tmp = 0;
  place_integral_type(src, tmp, i);
  dst = (tmp != 0);
}

// Specialization for when SrcT == DstT
template<typename T>
void place_integral_type(const T src, T & dst, const size_t i) {
  assert(i == 0);
  dst = src;
}

}

#endif  // CMBML__PLACE_INTEGRAL_TYPE__HPP_
