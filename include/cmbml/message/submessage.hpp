#ifndef CMBML__SUBMESSAGE__HPP_
#define CMBML__SUBMESSAGE__HPP_

#include <vector>
#include <cinttypes>

#include <cmbml/types.hpp>
#include <boost/hana/define_struct.hpp>

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


  enum SubmessageKind : uint8_t {
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
    BOOST_HANA_DEFINE_STRUCT(SubmessageHeader,
      (SubmessageKind, submessage_id),
      (std::array<SubmessageFlag, 8>, flags),
      (uint16_t, submessage_length)
    );
  };

  // TODO write a utility templated on the type of the message to correctly interpret flags
  // see page 169 of spec

  // Submessage Elements

  // Bitmap representation of a set of sequence numbers
  // For all elements in the set,
  // base <= element <= base+255
  template<typename T>
  struct IntegralSet {
    BOOST_HANA_DEFINE_STRUCT(IntegralSet,
      (T, base),
      (List<T>, set));
  };

  using SequenceNumberSet = IntegralSet<SequenceNumber_t>;
  using FragmentNumberSet = IntegralSet<FragmentNumber_t>;

  struct Parameter {
    BOOST_HANA_DEFINE_STRUCT(Parameter,
    (ParameterId_t, id),
    (List<Octet>, value));
  };

  // TODO Add a type trait so that only types with the "Subelement" Concept can specialize
  // (that is, all the structs defined in "data.hpp"
  template<typename SubmessageElement>
  struct Submessage {
    BOOST_HANA_DEFINE_STRUCT(Submessage,
      (SubmessageHeader, header),
      (SubmessageElement, element)
    );
  };


}


#endif  // CMBML__SUBMESSAGE__HPP_
