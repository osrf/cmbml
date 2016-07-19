// A utility for deserializing anything (in CDR format)
#ifndef CMBML__DESERIALIZER__HPP_
#define CMBML__DESERIALIZER__HPP_

#include <boost/hana/for_each.hpp>

#include <cmbml/common.hpp>
#include <cmbml/serialization/place_integral_type.hpp>

#include <cmbml/message/message.hpp>

#include <cmbml/utility/console_print.hpp>

namespace hana = boost::hana;

namespace cmbml {

// TODO Custom serialization rules for e.g. bool

// Data types register callbacks for themselves!

// I guess we need a bool specialization that doesn't pad and one that does for custom types
template<typename DstT, typename SrcT,
  typename std::enable_if<std::is_integral<DstT>::value>::type * = nullptr>
StatusCode deserialize(DstT & dst, const SrcT & src, size_t & index) {
  // "De-alignment" checks
  // We know that the value of dst will be aligned to a boundary which is a multiple of
  // its size. Predict the padding to skip based on that.
  if (index % number_of_bits<DstT>() != 0) {
    index += number_of_bits<DstT>() - (index % number_of_bits<DstT>());
  }

  if ((src.begin() + index / 32) >= src.end()) {
    return StatusCode::precondition_violated;
  }

  place_integral_type(src[index / 32], dst, index % 32);
  index += number_of_bits<DstT>();
  return StatusCode::ok;
}

template<typename DstT, typename SrcT,
  typename std::enable_if<std::is_enum<DstT>::value>::type * = nullptr>
StatusCode deserialize(DstT & dst, const SrcT & src, size_t & index)
{
  using UnderlyingT = typename std::underlying_type<DstT>::type;
  UnderlyingT tmp_dst;
  StatusCode ret = deserialize(tmp_dst, src, index);
  if (ret != StatusCode::ok) {
    return ret;
  }
  dst = static_cast<DstT>(tmp_dst);
  return StatusCode::ok;
}

template<typename SrcT, size_t NumBits>
StatusCode deserialize(std::bitset<NumBits> & dst, const SrcT & src, size_t & index)
{
  uint32_t flags = 0;
  StatusCode status = deserialize(flags, src, index);
  dst = std::bitset<NumBits>(flags);
  return status;
}

template<typename SrcT>
StatusCode deserialize(String & dst, const SrcT & src, size_t & index)
{
  uint32_t string_size;
  StatusCode status = deserialize(string_size, src, index);
  if (status != StatusCode::ok) {
    return status;
  }
  // String deserialization requires allocation if std::string is used.
  // Could make this more efficient with a custom string class.
  // char array
  dst.resize(string_size);
  for (auto & entry : dst) {
    status = deserialize(entry, src, index);
  }
  return status;
}

// Specialization for ParameterList
template<typename SrcT>
StatusCode deserialize(List<Parameter> & dst, const SrcT & src, size_t index)
{
  // TODO alignment
  size_t i = 0;
  StatusCode status = StatusCode::ok;
  while (status == StatusCode::ok) {
    ParameterId_t id;
    status = deserialize(id, src, index);
    if (id == ParameterId_t::sentinel || status != StatusCode::ok) {
      break;
    }
    uint16_t length = 0;
    status = deserialize(length, src, index);
    // Check if dst is big enough
    if (dst.size() <= i) {
      dst.resize(i + 1);
    }
    dst[i].id = id;
    dst[i].length = length;
    dst[i].value.resize(length);
    for (auto entry : dst[i].value) {
      status = deserialize(entry, src, index);
    }
  };
  uint16_t tmp;
  // expect to have 16bits of padding here, and that index is on a 32-bit boundary
  return deserialize(tmp, src, index);
}

// We always expect callback to take a single argument of the type 
// We currently expect dst to be preallocated (if dynamically sized)
template<typename T, typename SrcT, class = typename T::iterator>
StatusCode deserialize(T & dst, const SrcT & src, size_t & index)
{
  // Chain callback: we expect this to deserialize out the length of the array
  uint32_t array_length;
  StatusCode ret = deserialize(array_length, src, index);
  if (ret != StatusCode::ok) {
    return ret;
  }
  // assert(array_length == dst.size());
  if (array_length != dst.size()) {
    return StatusCode::precondition_violated;
  }
  for (auto & entry : dst) {
    ret = deserialize(entry, src, index);
    if (ret != StatusCode::ok) {
      return ret;
    }
  }
  return StatusCode::ok;
}


template<typename DstT, typename SrcT,
  typename std::enable_if<hana::Foldable<DstT>::value>::type * = nullptr>
StatusCode deserialize(DstT & dst, const SrcT & src, size_t & index) {
  StatusCode ret = StatusCode::ok;
  hana::for_each(dst, [&src, &index, &ret](auto dst_field) {
    auto dst_value = hana::second(dst_field);
    // Avoid more side effects if we encountered an error previously.
    if (ret == StatusCode::ok) {
      ret = deserialize(dst_value, src, index);
    }
  });
  return ret;
}

template<typename DstT, typename SrcT>
StatusCode deserialize(DstT & dst, const SrcT & src) {
  size_t i = 0;
  return deserialize(dst, src, i);
}

// We could get fancy and use the return type of the callback
template<typename DstT, typename SrcT, typename CallbackT, typename ...CallbackArgs>
StatusCode deserialize(
    const SrcT & src, size_t & index, CallbackT && callback, CallbackArgs &&...args)
{
  DstT dst;
  StatusCode ret = deserialize(dst, src, index);
  if (ret != StatusCode::ok) {
    return ret;
  }
  return callback(dst, std::forward<CallbackArgs>(args)...);
}

}

#endif  // CMBML__DESERIALIZER__HPP_
