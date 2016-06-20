#ifndef CMBML__SUBMESSAGE__HPP_
#define CMBML__SUBMESSAGE__HPP_

#include <memory>
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
    pad = 0x1,
    acknack = 0x06,
    heartbeat = 0x07,
    gap = 0x08,
    info_ts = 0x09,
    info_src = 0x0c,
    info_reply_ip4 = 0x0d,  // TODO need to propagate ipv4 vs. ipv6 to locator_list?
    info_dst = 0x0e,
    info_reply = 0x0f,  // TODO need to propagate ipv4 vs. ipv6 to locator_list?
    nack_frag = 0x12,
    heartbeat_frag = 0x13,
    data = 0x15,
    data_frag = 0x16
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

  struct SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(SubmessageElement,
      (Endianness, endianness_flag)
    );

    virtual ~SubmessageElement() {};
  };

  struct Submessage {
    BOOST_HANA_DEFINE_STRUCT(Submessage,
      (SubmessageHeader, header),
      (std::unique_ptr<SubmessageElement>, element)
    );

    Submessage() {
    }

    explicit Submessage(SubmessageElement * element_ptr) {
      element.reset(element_ptr);
    }
    explicit Submessage(std::unique_ptr<SubmessageElement> element_ptr) {
      element.reset(element_ptr.get());
    }
  };

}


#endif  // CMBML__SUBMESSAGE__HPP_
