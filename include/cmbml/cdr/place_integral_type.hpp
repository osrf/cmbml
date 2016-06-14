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
template<
  typename SrcT,
  typename DstT,
  typename std::enable_if<std::is_integral<SrcT>::value &&
      std::is_integral<DstT>::value &&
      !std::is_same<SrcT, bool>::value && !std::is_same<DstT, bool>::value
      >::type * = nullptr>
void place_integral_type(const SrcT src, DstT & dst, size_t & i) {
  dst = 0;
  if (sizeof(SrcT) <= sizeof(DstT)) {

    // We need to fit something of size src into the long at the specified index
    // We expect src to span from index i to index "i + sizeof(IntegralType)"
    assert(i <= number_of_bits<DstT>() - number_of_bits<SrcT>());

    DstT widened_src = src;
    // Bitshift src to the index which it will land at 
    widened_src = widened_src << i;

    // zero the part of dst which we plan to replace
    DstT mask = max_value_map[hana::type_c<DstT>];
    mask = mask << i;  // produces zeros until the first index
    mask |= max_value_map[hana::type_c<DstT>] >> (i + number_of_bits<SrcT>());

    dst &= mask;
    // OR dst with the widened src
    dst |= src;
    //i = (i + number_of_bits<SrcT>()) % number_of_bits<DstT>();
    i += number_of_bits<SrcT>();
  } else {  // Narrow
    assert(i <= number_of_bits<SrcT>() - number_of_bits<DstT>());
    // i refers to the index of src from which we will extract the bits to place into dst
    // Make a copy of src, bitshift, then mask
    SrcT copy = src;
    // 
    copy = copy >> (number_of_bits<SrcT>() - (i + number_of_bits<DstT>()));
    SrcT mask = max_value_map[hana::type_c<SrcT>];
    mask = mask >> (number_of_bits<SrcT>() - number_of_bits<DstT>());
    copy &= mask;
    dst = copy;
    i += number_of_bits<DstT>();
  }
}


// Bool specialization for flags
// first of all, partial specialization doesn't work :(
// Need a separate specialization for metadata/flags vs data serialization for uint8 size
template<typename DstT>
void place_integral_type(const bool src, DstT & dst, size_t & i) {
  assert(i < number_of_bits<DstT>());
  DstT mask = 1;
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

template<typename SrcT>
void place_integral_type(const SrcT src, bool & dst, size_t & i) {
  assert(i < number_of_bits<SrcT>());
  // TODO Reverse
  SrcT copy = src;
  copy = copy << i++;
  copy = copy >> (number_of_bits<SrcT>() - 1);
  dst = copy;
}



// Specialization for when SrcT == DstT
template<typename T>
void place_integral_type(const T src, T & dst, size_t & i) {
  assert(i == 0);
  dst = src;
}

}

#endif  // CMBML__PLACE_INTEGRAL_TYPE__HPP_
