// A utility for serializing anything (in CDR format)
#ifndef CMBML__SERIALIZER__HPP_
#define CMBML__SERIALIZER__HPP_

#include <boost/hana/tuple.hpp>

#include <cassert>

namespace cmbml {

// Must enforce alignment: we always align to a 32-bit boundary
// and we never split a struct across that boundary

// Endianness
// Endianness of serialization SHOULD be configurable
// But in the current draft we assume the following:
//
// Suppose an array of uint8_t looks like this:
// index:    0    1    2    3
// contents: 0x1, 0x2, 0x3, 0x4 ...
//
// it would compose into single long of value: 0x10203040
//
// Here's example from the RTPS spec.
// higher index is less significant bit.
// byte 0          | byte 1        | byte 2       | byte 3
// 0...2...........7...............15.............23...............31
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    ACKNACK    |X|X|X|X|X|X|F|E|        octetsToNextHeader
//


// if the type has a fixed size we know exactly how many bytes will be needed at compile time.
// Decompose structs into tuples of primitive types (arrays of integral types)

// TODO: deduce message length from T, can this be done at compile time even with padding?
// I contend that yes it can
template<typename T, size_t MessageLength>
std::array<uint32_t, MessageLength> serialize_message(const T & element);

template<typename ...TupleArgs, size_t MessageLength>
std::array<uint32_t, MessageLength> serialize(boost::hana::tuple<TupleArgs...> & element)
{
  
}

// base case while serializing: primitive types
// template<typename >

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
void place_integral_type(const IntegralType src, uint32_t & dst, size_t i) {
  // We must be able to fit the src type into dst
  static_assert(
      sizeof(IntegralType) <= sizeof(uint32_t),
      "Couldn't fit serialized integral type into a uint32_t!");

  // We need to fit something of size src into the long at the specified index
  // We expect src to span from index i to index "i + sizeof(IntegralType)"
  assert(i < sizeof(uint32_t) - sizeof(IntegralType));

  uint32_t widened_src = src;
  // Bitshift src to the index which it will land at 
  widened_src = widened_src << i;

  // zero the part of dst which we plan to replace
  uint32_t mask = UINT32_MAX;
  mask = mask << i;  // produces zeros until the first index
  mask |= UINT32_MAX >> (i + sizeof(IntegralType));

  dst &= mask;
  // OR dst with the widened src
  dst |= src;
}

// Specialization for uint32_t
template<>
void place_integral_type(const uint32_t src, uint32_t & dst, size_t i) {
  assert(i == 0);
  dst = src;
}

// Bool specialization for flags (TODO)
template<>
void place_integral_type(const bool src, uint32_t & dst, size_t i) {

}

// Specializations for primitive types
// Concepts: SrcT and DestT are Integral types with the same signedness
template<typename DstT, typename SrcT, size_t SrcLength,
  typename std::enable_if<sizeof(DstT) < sizeof(SrcT)*SrcLength >::type * = nullptr>
std::array<DstT, sizeof(SrcT)*SrcLength/sizeof(DstT)> convert_representations(
    const std::array<SrcT, SrcLength> & src);

// Specialization if the representation of the source type is smaller than the destination type
// TODO Unit test me
template<typename DstT, typename SrcT, size_t SrcLength,
  typename std::enable_if<sizeof(SrcT) <= sizeof(DstT) - 1 >::type * = nullptr>
std::array<DstT, sizeof(SrcT)*SrcLength/sizeof(DstT)> convert_representations(
    const std::array<SrcT, SrcLength> & src)
{
  using ReturnType = std::array<DstT, sizeof(SrcT)*SrcLength/sizeof(DstT)>;
  // How many SrcT's can we fit into a DstT?
  const size_t pack_count = sizeof(DstT) % sizeof(SrcT);

  size_t src_index = 0;

  ReturnType dst;
  dst.fill(0);

  for (auto entry : dst) {
    // Place the lower indices into the most significant bit.
    for (size_t i = 0; i < pack_count; ++i) {
      entry |= src[src_index + i] << (pack_count - 1 - i)*sizeof(SrcT);
    }
    src_index += pack_count;
  }

  return dst;
}

// Specialization if the representation of the destination type is smaller than the source type
// TODO Unit test me
template<typename DstT, typename SrcT, size_t SrcLength,
  typename std::enable_if<sizeof(DstT) <= sizeof(SrcT) - 1 >::type * = nullptr>
std::array<DstT, sizeof(SrcT)*SrcLength/sizeof(DstT)> convert_representations(
    const std::array<SrcT, SrcLength> & src)
{
  using ReturnType = std::array<DstT, sizeof(SrcT)*SrcLength/sizeof(DstT)>;
  // How many DstT's are split from a SrcT?
  const size_t unpack_count = sizeof(SrcT) % sizeof(DstT);

  ReturnType dst;
  dst.fill(0);

  size_t dst_index = 0;

  for (auto entry : src) {
    for (size_t i = 0; i < unpack_count; ++i) {
      // MSB of entry goes into the lower index of dst
      // TODO this is wrong
      dst[dst_index + i] = entry >> (unpack_count - 1 - i)*sizeof(DstT);
    }
    dst_index += unpack_count;
  }

  return dst;
}

}

#endif  // CMBML__SERIALIZER__HPP_
