// A utility for serializing anything (in CDR format)
#ifndef CMBML__SERIALIZER__HPP_
#define CMBML__SERIALIZER__HPP_

#include <boost/hana/pair.hpp>
#include <boost/hana/eval_if.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/fwd/flatten.hpp>

#include <algorithm>
#include <cmath>
#include <bitset>
#include <cassert>
#include <climits>
#include <functional>

#include <cmbml/cdr/common.hpp>
#include <cmbml/cdr/place_integral_type.hpp>
#include <cmbml/types.hpp>  // Provides "List" type

#include <cmbml/message/data.hpp>
#include <cmbml/message/submessage.hpp>
#include <cmbml/message/message.hpp>

// What if we used bitsets instead?
// What is their performance like?

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

// For all functions,
// index is the overall bitwise index into the packet.
// it's going to be simpler this way
// Access is: packet[floor(index, 32)] and 

template<typename T, typename CallbackT,
  typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
void traverse(const T element, CallbackT && callback, size_t & index)
{
  // Observe CDR alignment rule: a primitive must be aligned on an index which is
  // a multiple of its length.
  if (index % number_of_bits<T>() != 0) {
    // Pad subindex
    index += number_of_bits<T>() - (index % number_of_bits<T>());
  }

  callback(element);
  // place_integral_type(element, dst[index / 32], index % 32);
  index += number_of_bits<T>();
}

template<typename T, typename CallbackT,
  typename std::enable_if<std::is_enum<T>::value>::type * = nullptr>
void traverse(const T element, CallbackT && callback, size_t & index)
{
  traverse(static_cast<typename std::underlying_type<T>::type>(element), callback, index);
}

// TODO Specialization for ParameterList. Parameters currently do not conform to CDR spec

// TODO Specialization for LocatorList. does not conform to CDR spec

// TODO sometimes we serialize the length of variable-length lists and sometimes we don't;
// clarify when this happens.
// this is for std::vector and std::array of non-primitive types
template<typename ContainerT, typename CallbackT,
  class = typename ContainerT::iterator>  // Enable specialization if ContainerT is Iterable
void traverse(
    const ContainerT & src,
    CallbackT & callback,
    size_t & index)
{
  traverse(static_cast<uint32_t>(src.size()), callback, index);
  for (const auto & element : src) {
    traverse(element, callback, index);
  }
}

template<typename ... TupleArgs, typename CallbackT>
void traverse(
    const hana::tuple<TupleArgs...> & element,
    CallbackT && callback,
    size_t & index)
{
  hana::for_each(element, [&callback, &index](const auto x){
    traverse(x, callback, index);
  });
}

template<
  typename T, typename CallbackT,
  typename std::enable_if<hana::Foldable<T>::value>::type * = nullptr>
void traverse(
    const T & element,
    CallbackT && callback,
    size_t & index)
{
  hana::for_each(element, [&callback, &index](const auto x){
    traverse(hana::second(x), callback, index);
  });
}

template<typename T>
size_t get_packet_size(const T & element) {
  size_t index = 0;
  auto count_packet_size = [&index](auto element) {
    (void) element;
  };
  traverse(element, count_packet_size, index);
  return index + 1;
}

template<typename T, typename DstT>
void serialize(const T & element, DstT & dst, size_t & index) {
  auto serialize_base_case = [&dst, &index](auto element) {
    place_integral_type(element, dst[index / 32], index % 32);
  };
  traverse(element, serialize_base_case, index);
}


// Convenience function because we cannot assign a default value to an lvalue reference
template<typename T, typename DstT = Packet<>>
void serialize(const T & element, DstT & dst)
{
  /* how can we be smartest about preallocating?
   * We can't predict the size of the packet until serialize is called.
   * We require dst to be zeroed and preallocated before this function is called*/
  size_t index = 0;  // represents the bitwise index
  serialize(element, dst, index);
}

}

#endif  // CMBML__SERIALIZER__HPP_
