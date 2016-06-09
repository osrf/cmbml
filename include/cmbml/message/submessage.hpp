#ifndef CMBML__SUBMESSAGE__HPP_
#define CMBML__SUBMESSAGE__HPP_

#include <vector>
#include <cinttypes>

#include <cmbml/types.hpp>

namespace cmbml {


  using Count_t = uint32_t;
  using FragmentNumber_t = uint32_t;
  using SubmessageFlag = bool;
  using ParameterId_t = uint16_t;
  using Timestamp = Time_t;

  using FinalFlag = SubmessageFlag;
  using InlineQosFlag = SubmessageFlag;
  using DataFlag = SubmessageFlag;
  using KeyFlag = SubmessageFlag;
  using LivelinessFlag = SubmessageFlag;
  using MulticastFlag = SubmessageFlag;
  // If Invalidate is not set to true,
  // using InvalidateFlag = SubmessageFlag;


  // TODO dynamically sized?
  /*
  template<uint32_t size>
  using SerializedData = std::array<Octet, size>;

  template<uint32_t size>
  using SerializedDataFragment = std::array<Octet, size>;
  */
  using SerializedData = std::vector<Octet>;  // hmmmm

  using SerializedDataFragment = std::vector<Octet>;


  enum class SubmessageKind {
    data, gap, heartbeat, acknack, pad, info_ts, info_reply,
    info_dst, info_src, data_frag, nack_frag, heartbeat_frag
  };

  enum Endianness : bool {
    big_endian = false, little_endian = true
  };
  enum InvalidateFlag : bool {
    has_timestamp = false, no_timestamp = true
  };

  struct SubmessageHeader {
    SubmessageKind submessage_id;
    std::array<SubmessageFlag, 8> flags;
    uint16_t submessage_length;
  };

  // TODO write a utility templated on the type of the message to correctly interpret flags
  // see page 169 of spec

  // Submessage Elements

  // Bitmap representation of a set of sequence numbers
  template<typename T>
  struct IntegralSet {
    T base;
    // For all elements in the set,
    // base <= element <= base+255
    List<T> set;
  };

  using SequenceNumberSet = IntegralSet<SequenceNumber_t>;
  using FragmentNumberSet = IntegralSet<FragmentNumber_t>;

  struct ParameterIt {
  };

  template<uint16_t Length>
  struct Parameter : ParameterIt {
    ParameterId_t id;
    static const uint16_t length = Length;
    std::array<Octet, length> value;
  };

  template<typename SubmessageElement>
  struct Submessage {
    SubmessageHeader header;
    SubmessageElement element;
  };


}


#endif  // CMBML__SUBMESSAGE__HPP_
