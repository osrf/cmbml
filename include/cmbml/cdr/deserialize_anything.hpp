// A utility for deserializing anything (in CDR format)
#ifndef CMBML__DESERIALIZER__HPP_
#define CMBML__DESERIALIZER__HPP_

#include <boost/hana/for_each.hpp>

#include <cmbml/cdr/common.hpp>
#include <cmbml/cdr/place_integral_type.hpp>

#include <cmbml/message/message.hpp>

namespace hana = boost::hana;

namespace cmbml {


// Data types register callbacks for themselves!

// I guess we need a bool specialization that doesn't pad and one that does for custom types
template<typename DstT, typename SrcT,
  typename std::enable_if<std::is_integral<DstT>::value>::type * = nullptr>
void deserialize(DstT & dst, const SrcT & src, size_t & index) {
  // "De-alignment" checks
  // We know that the value of dst will be aligned to a boundary which is a multiple of
  // its size. Predict the padding to skip based on that.
  if (index % number_of_bits<DstT>() != 0) {
    index += number_of_bits<DstT>() - (index % number_of_bits<DstT>());
  }

  assert(src.begin() + (index / 32) < src.end());

  place_integral_type(src[index / 32], dst, index % 32);
  index += number_of_bits<DstT>();
}

template<typename DstT, typename SrcT,
  typename std::enable_if<std::is_enum<DstT>::value>::type * = nullptr>
void deserialize(DstT & dst, const SrcT & src, size_t & index)
{
  using UnderlyingT = typename std::underlying_type<DstT>::type;
  UnderlyingT tmp_dst;
  deserialize(tmp_dst, src, index);
  dst = static_cast<DstT>(tmp_dst);
}


// TODO won't work for vec
// We always expect callback to take a single argument of the type 
template<typename T, typename SrcT, class = typename T::iterator>
void deserialize(T & dst, const SrcT & src, size_t & index)
{
  // Chain callback: we expect this to deserialize out the length of the array
  uint32_t array_length;
  deserialize(array_length, src, index);
  assert(array_length == dst.size());
  for (auto & entry : dst) {
    deserialize(entry, src, index);
  }
}


template<typename DstT, typename SrcT,
  typename std::enable_if<hana::Foldable<DstT>::value>::type * = nullptr>
void deserialize(DstT & dst, const SrcT & src, size_t & index) {
  hana::for_each(dst, [&src, &index](auto dst_field) {
    auto dst_value = hana::second(dst_field);
    deserialize(dst_value, src, index);
  });
}

template<typename DstT, typename SrcT,  typename CallbackT, typename ...CallbackArgs>
void deserialize(const SrcT & src, size_t & index, CallbackT && callback, CallbackArgs &&...args) {
  DstT dst;
  deserialize(dst, src, index);
  callback(dst, std::forward<CallbackArgs>(args)...);
}

}

#endif  // CMBML__DESERIALIZER__HPP_
