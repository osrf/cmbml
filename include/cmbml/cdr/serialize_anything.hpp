// A utility for serializing anything (in CDR format)
#ifndef CMBML__SERIALIZER__HPP_
#define CMBML__SERIALIZER__HPP_

#include <boost/hana/pair.hpp>
#include <boost/hana/eval_if.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/fwd/flatten.hpp>

#include <algorithm>
#include <cassert>
#include <climits>
#include <functional>

#include <cmbml/cdr/convert_representations.hpp>
#include <cmbml/cdr/place_integral_type.hpp>
#include <cmbml/cdr/template_utilities.hpp>
#include <cmbml/types.hpp>  // Provides "List" type

namespace hana = boost::hana;

namespace cmbml {

// Must enforce alignment: we always align to a 32-bit boundary
// never split a field across a 32-bit boundary

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

// Signedness
// TODO verify if these functions are generalizable (make sure there are no weird conversion
// errors, possibly catch using numeric_limits::is_signed

// TODO CDR should be able to serialize 64-bit types.

// if the type has a fixed size we know exactly how many bytes will be needed at compile time.
// Decompose structs into tuples of primitive types (arrays of integral types)

// On alignment: 

// TODO: deduce message length from T, can this be done at compile time even with padding?
// Yes, for fixed-size messages
// Compile-time recursion is expensive
// However we don't expect the serialized elements to be deeply nested (constant, max of ~3 levels)
// 
// IMPORTANT: We assume that dst is zero-initialized when receive it
// maybe the initial serialize helper function should zero-initialize it.
template<
  typename T,
  typename DstT,
  typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
void serialize(const T element, DstT & dst,
    size_t & index,
    size_t & subindex)
{
  // Observe CDR alignment rule: primitives must be aligned on their length.
  if (auto remainder = subindex % number_of_bits<T>() != 0) {
    // Pad subindex
    subindex += remainder;
  }

  if (subindex + number_of_bits<T>() >= number_of_bits<uint32_t>()) {
    ++index;
    subindex = 0;
  }
  place_integral_type(element, dst[index], subindex);
}

template<
  typename T,
  typename DstT,
  typename std::enable_if<std::is_enum<T>::value>::type * = nullptr>
void serialize(const T element, DstT & dst,
    size_t & index,
    size_t & subindex)
{
  serialize(static_cast<typename std::underlying_type<T>::type>(element), dst, index, subindex);
}

template<typename T, size_t ArraySize, typename DstT,
  typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type * = nullptr>
void serialize(
    const std::array<T, ArraySize> & element,
    DstT & dst,
    size_t & index,
    size_t & subindex)
{
  if (auto remainder = subindex % number_of_bits<T>() != 0) {
    subindex += remainder;
  }

  if (subindex + number_of_bits<T>() >= number_of_bits<uint32_t>()) {
    ++index;
    subindex = 0;
  }
  convert_representations(element, dst, dst.begin() + index);
  index += sizeof(T)*ArraySize/sizeof(uint32_t);

  subindex = number_of_bits<T>() % number_of_bits<uint32_t>();
}

// TODO sometimes we serialize the length of variable-length lists and sometimes we don't;
// clarify when this happens.
// this is for std::vector and std::array of non-primitive types
template<typename ContainerT, typename DstT,
  class = typename ContainerT::iterator>  // Enable specialization if ContainerT is Iterable
void serialize(
    const ContainerT & src,
    DstT & dst,
    size_t & index,
    size_t & subindex)
{
  for (const auto & element : src) {
    serialize(element, dst, index, subindex);
  }
}

template<
  typename DstT,
  typename ... TupleArgs>
void serialize(
    const hana::tuple<TupleArgs...> & element,
    DstT & dst,
    size_t & index,
    size_t & subindex)
{
  hana::for_each(element, [&dst, &index, &subindex](const auto x){
    serialize(x, dst, index, subindex);
  });
}

template<
  typename T,
  typename DstT,
  typename std::enable_if<hana::Foldable<T>::value>::type * = nullptr>
void serialize(
    const T & element,
    DstT & dst,
    size_t & index,
    size_t & subindex)
{
  hana::for_each(element, [&dst, &index, &subindex](const auto x){
    serialize(hana::second(x), dst, index, subindex);
  });
}

// Convenience function because we cannot assign a default value to an lvalue reference
template<typename T, typename DstT>
void serialize(const T & element, DstT & dst)
{
  size_t index = 0;
  size_t subindex = 0;
  serialize(element, dst, index, subindex);
}

}

#endif  // CMBML__SERIALIZER__HPP_
