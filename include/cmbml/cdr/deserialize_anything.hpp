// A utility for deserializing anything (in CDR format)
#ifndef CMBML__DESERIALIZER__HPP_
#define CMBML__DESERIALIZER__HPP_

#include <boost/hana/for_each.hpp>

#include <cmbml/cdr/place_integral_type.hpp>

#include <cmbml/message/message.hpp>

namespace hana = boost::hana;

namespace cmbml {

// Data types register callbacks for themselves!
// 

template<typename SrcT, typename DstT, typename CallbackT
  typename std::enable_if<std::is_integral<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, size_t & index, CallbackT && callback)
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

template<typename SrcT, typename DstT, typename CallbackT
  typename std::enable_if<std::is_enum<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, size_t & index, size_t & subindex, CalbackT && callback)
{
  using IntType = typename std::underlying_type<DstT>::type;
  IntType dstint = 0;
  deserialize(src, index, subindex);
  dst = static_cast<DstT>(dstint);
}

template<typename SrcT, typename T, size_t ArraySize, typename CallbackT,
  typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type * = nullptr>
void deserialize(
  const SrcT & src, size_t & index, size_t & subindex, CallbackT && callback)
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
  typename T,
  typename CallbackT>
void deserialize(
    const SrcT & src,
    List<T> & dst,
    size_t & index,
    size_t & subindex,
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
  typename std::enable_if<std::is_integral<DstT>::value>::type * = nullptr >
void deserialize(const SrcT & src, size_t & index, size_t & subindex, CallbackT && callback) {
  // Deserialize an integer out of src
  // but put it where?
}


template<typename DstT, typename SrcT, typename CallbackT
  typename std::enable_if<hana::Foldable<T>::value>::type * = nullptr>
void deserialize(const SrcT & src, size_t & index, CallbackT && callback) {
   // Dst gives us the structure of the fields
   // Once the data is deserialized, call the registered callback for Dst
   // can I unfold a type_c?
  hana::for_each(hana::type_c<DstT>, [](auto & x) {
      deserialize<std::decay_t<decltype(x)>>(src, index, callback);
  });
}

#define CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(Type) \
  { \
    auto chaining_callback = [callback](auto & x) { \
      registered_callbacks[hana::type_c<Type>](x); \
      /*TODO args*/ \
      callback(); \
    } \
    deserialize<Type>(src, index, chaining_callback); \
  } \


template<typename SrcT, typename CallbackT>
void deserialize<SubmessageHeader>(
  const SrcT & src, size_t & index, CallbackT && callback)
{
  // Could be phrased more functionally if, once I figure out how callbacks chain via lambdas,
  // if this were done by traversing the Submessage type?
  deserialize<SubmessageHeader>(src, index);

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
template<typename SrcT, typename CallbackT>
void deserialize_message(const SrcT & src,
    size_t & index, CallbackT && callback)
{
  // Header deserialization
  // We know there's a header at the start of the message.
  // TODO How do we chain callbacks so that the state of Header remains?
  // TODO How do we copy data out of deserialize?
  deserialize<Header>(src, index, callback);
  // TODO Need to know the end condition from src
  // (need to take "end predicate")?
  deserialize<SubmessageHeader>(src, index, callback);
}

// So I guess it would be really cool if the callbacks incremented the state of the state machine.
template<typename SrcT, typename CallbackT>
void deserialize_message(const SrcT & src, CallbackT && callback)
{
  size_t index = 0;
  deserialize_message(src, index, callback);
}

}

#endif  // CMBML__DESERIALIZER__HPP_
