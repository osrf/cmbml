// A utility for deserializing anything (in CDR format)
#ifndef CMBML__DESERIALIZER__HPP_
#define CMBML__DESERIALIZER__HPP_

#include <boost/hana/for_each.hpp>

#include <cmbml/cdr/place_integral_type.hpp>

#include <cmbml/message/message.hpp>

namespace hana = boost::hana;

namespace cmbml {

template<typename SrcT, typename DstT,
  typename std::enable_if<std::is_integral<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, DstT & dst, size_t & index, size_t & subindex)
{
  // "De-alignment" checks
  // We know that the value of dst will be aligned to a boundary which is a multiple of
  // its size. Predict the padding to skip based on that.
  if (auto remainder = index % number_of_bits<DstT>() != 0) {
    subindex += remainder;
  }

    // TODO: still assumes DstT is 32 bits
  if (subindex + number_of_bits<DstT>() >= number_of_bits<uint32_t>()) {
    ++index;
    subindex = 0;
  }

  assert(src.begin() + index < src.end());

  place_integral_type(src[index], dst, subindex);
  if (subindex >= number_of_bits<uint32_t>()) {
    // We went over a boundary
    index++;
    subindex = 0;
  }
}

template<typename SrcT, typename DstT,
  typename std::enable_if<std::is_enum<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, DstT & dst, size_t & index, size_t & subindex)
{
  using IntType = typename std::underlying_type<DstT>::type;
  IntType dstint = 0;
  deserialize(src, dstint, index, subindex);
  dst = static_cast<DstT>(dstint);
}

template<typename SrcT, typename T, size_t ArraySize,
  typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type * = nullptr>
void deserialize(
  const SrcT & src, std::array<T, ArraySize> & dst, size_t & index, size_t & subindex)
{
  if (auto remainder = subindex % number_of_bits<T>() != 0) {
    subindex += remainder;
  }

  if (subindex + number_of_bits<T>() >= number_of_bits<uint32_t>()) {
    ++index;
    subindex = 0;
  }

  // We think this works ok...
  for (auto & entry : dst) {
    place_integral_type(src[index], entry, subindex);

    if (subindex + number_of_bits<T>() >= number_of_bits<uint32_t>()) {
      // We went over a boundary
      index++;
      subindex = 0;
    }
  }
}

// Precondition: dst is empty
template<
  typename SrcT,
  typename T>
void deserialize(
    const SrcT & src,
    List<T> & dst,
    size_t & index,
    size_t & subindex)
{
   uint32_t size = 0;
   deserialize(src, size, index, subindex);
   for (uint32_t i = 0; i < size; ++i) {
     T element;
     deserialize(src, element, index, subindex);
     dst.push_back(element);
   }
}

//  TODO: Add a specialization for ParameterList, which terminates with a sentinel

template<
  typename SrcT,
  typename ... TupleArgs>
void deserialize(
    const SrcT & src,
    hana::tuple<TupleArgs...> & dst,
    size_t & index,
    size_t & subindex)
{
  hana::for_each(dst, [src, &index, &subindex](auto x){
    deserialize(src, x, index, subindex);
  });
}


template<
  typename SrcT,
  typename T,
  typename std::enable_if<hana::Foldable<T>::value>::type * = nullptr>
void deserialize(
    const SrcT & src,
    T & dst,
    size_t & index,
    size_t & subindex)
{
  hana::for_each(dst, [src, &index, &subindex](auto x){
    deserialize(src, hana::second(x), index, subindex);
  });
}

#define CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(Type) \
  { \
    auto * element = static_cast<const Type *>(dst.element.get()); \
    assert(element); \
    deserialize(src, *element, index, subindex); \
  } \


template<typename SrcT>
void deserialize(const SrcT & src, Submessage & dst, size_t & index, size_t & subindex) {
  deserialize(src, dst.header, index, subindex);

  switch (dst.header.submessage_id) {
    case SubmessageKind::acknack_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(AckNack);
      break;
    case SubmessageKind::heartbeat_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(Heartbeat);
      break;
    case SubmessageKind::gap_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(Gap);
      break;
    case SubmessageKind::info_ts_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(InfoTimestamp);
      break;
    case SubmessageKind::info_src_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(InfoSource);
      break;
    case SubmessageKind::info_reply_ip4_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(InfoReply);
      break;
    case SubmessageKind::info_reply_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(InfoReply);
      break;
    case SubmessageKind::info_dst_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(InfoDestination);
      break;
    case SubmessageKind::nack_frag_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(NackFrag);
      break;
    case SubmessageKind::heartbeat_frag_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(HeartbeatFrag);
      break;
    case SubmessageKind::data_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(Data);
      break;
    case SubmessageKind::data_frag_id:
      CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(DataFrag);
      break;
    default:
      assert(false);
  }
}

// In all deserialize methods, the packet provides its own length.
// If index ever exceeds the payload length, exit.
// TODO dynamic allocation here, think about arena allocators
// Can we preallocate by looking at the raw payload before allocating
// its representation?
// Precondition: assumes Message.messages is empty
template<size_t PayloadLength>
void deserialize(const std::array<uint32_t, PayloadLength> & src,
    Message & dst, size_t & index, size_t & subindex)
{
  deserialize(src, dst.header, index, subindex);
  // TODO Hint about payload length
  while (index < PayloadLength) {
    Submessage submessage;
    deserialize(src, submessage, index, subindex);
    dst.messages.push_back(std::move(submessage));
  }
}

template<typename SrcT, typename DstT>
void deserialize(const SrcT & src, DstT & dst)
{
  size_t index = 0;
  size_t subindex = 0;
  deserialize(src, dst, index, subindex);
}

}

#endif  // CMBML__DESERIALIZER__HPP_
