// A utility for deserializing anything (in CDR format)
#ifndef CMBML__DESERIALIZER__HPP_
#define CMBML__DESERIALIZER__HPP_

#include <boost/hana/for_each.hpp>

#include <cmbml/cdr/common.hpp>

#include <cmbml/message/message.hpp>

namespace hana = boost::hana;

namespace cmbml {

// Data types register callbacks for themselves!

// I guess we need a bool specialization that doesn't pad and one that does for custom types
template<typename DstT, typename SrcT, typename CallbackT, typename ...CallbackArgs,
  typename std::enable_if<std::is_integral<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, size_t & index, CallbackT && callback, CallbackArgs &&...args)
{
  // "De-alignment" checks
  // We know that the value of dst will be aligned to a boundary which is a multiple of
  // its size. Predict the padding to skip based on that.
  DstT dst;
  if (index % number_of_bits<DstT>() != 0) {
    index += number_of_bits<DstT>() - (index % number_of_bits<DstT>());
  }

  assert(src.begin() + (index / 32) < src.end());

  place_integral_type(src[index / 32], dst, index % 32);
  index += number_of_bits<DstT>();
  callback(dst, std::forward<CallbackArgs>(args)...);
}

template<typename DstT, typename SrcT, typename CallbackT,
  typename std::enable_if<std::is_enum<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, size_t & index, CallbackT && callback)
{
  deserialize<typename std::underlying_type<DstT>::type>(src, index, callback);
}

// TODO This ain't gonna work for vec
// We always expect callback to take a single argument of the type 
template<typename T, typename SrcT, typename CallbackT,
  typename T::iterator * = nullptr>
void deserialize(
  const SrcT & src, size_t & index, CallbackT && callback)
{
  uint32_t array_length;
  auto array_length_callback = [&array_length](uint32_t deserialized_length) {
    // We may need to assert that the deserialized length is ArraySize
    // Due to the type specification we actually don't need to know this?
    array_length = deserialized_length;
  };
  // Chain callback: we expect this to deserialize out the length of the array
  deserialize<uint32_t>(src, index, array_length_callback);
  T deserialized_array;
  auto array_entry_callback = [&deserialized_array](typename T::value_type entry, const uint32_t i) {
    deserialized_array[i] = entry;
  };
  for (uint32_t i = 0; i < deserialized_array.size(); ++i) {
    deserialize<typename T::value_type>(src, index, array_entry_callback, i);
  }
  callback(deserialized_array);
}

// Precondition: dst is empty
/*
template<
  typename SrcT,
  typename T,
  typename CallbackT>
void deserialize(
    const SrcT & src,
    List<T> & dst,
    size_t & index,
    CallbackT callback)
{
   uint32_t size = 0;
   deserialize(src, size, index, subindex);
   for (uint32_t i = 0; i < size; ++i) {
     T element;
     deserialize(src, element, index, subindex);
     dst.push_back(element);
   }
}
*/

//  TODO: Add a specialization for ParameterList, which terminates with a sentinel

/*
template<
  typename SrcT,
  typename CallbackT,
  typename ... TupleArgs>
void deserialize(
    const SrcT & src,
    size_t & index,
    size_t & subindex,
    CallbackT && callback)
{
  hana::for_each(<typename>, [src, &index, &subindex](auto x){
    deserialize(src, x, index, subindex, callback);
  });
}
*/

template<typename DstT, typename SrcT, typename CallbackT,
  typename std::enable_if<hana::Foldable<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, size_t & index, CallbackT && callback) {
  DstT dst;
  hana::for_each(dst, [&src, &index, &callback](auto dst_field) {
    auto dst_value = hana::second(dst_field);
    using DstValueT = std::decay_t<decltype(dst_value)>;
    auto transfer = [&dst_value](auto & serialized_value) {
      dst_value = static_cast<DstValueT>(serialized_value);
    };
    deserialize<DstValueT>(src, index, transfer);
  });
  callback(dst);
}

// In all deserialize methods, the packet provides its own length.
// If index ever exceeds the payload length, exit.
// TODO dynamic allocation here, think about arena allocators
// Can we preallocate by looking at the raw payload before allocating
// its representation?
// Precondition: assumes Message.messages is empty
/*
template<typename SrcT, typename CallbackT>
void deserialize_message(const SrcT & src,
    size_t & index, CallbackT && callback)
{
  // Header deserialization
  // We know there's a header at the start of the message.
  // TODO consider taking an "end predicate"
  deserialize<Header>(src, index, callback);
  while (index < src.size()) {
    // There is a possible error condition here where the remaining space of the packet
    // does not the size of the submessage
    deserialize_submessage(src, index, callback);
  }
}
*/

// So I guess it would be really cool if the callbacks incremented the state of the state machine.
/*
template<typename SrcT, typename CallbackT>
void deserialize_message(const SrcT & src, CallbackT && callback)
{
  size_t index = 0;
  // OK here's what's up
  // We deserialize the callback and 
  deserialize_message(src, index, callback);
}
*/

}

#endif  // CMBML__DESERIALIZER__HPP_
