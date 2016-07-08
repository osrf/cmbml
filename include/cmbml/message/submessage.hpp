#ifndef CMBML__SUBMESSAGE__HPP_
#define CMBML__SUBMESSAGE__HPP_

#include <memory>
#include <vector>
#include <cinttypes>

#include <bitset>

#include <cmbml/types.hpp>
#include <boost/hana/define_struct.hpp>

namespace cmbml {


  using Count_t = uint32_t;
  using FragmentNumber_t = uint32_t;
  using SubmessageFlag = bool;
  using ParameterId_t = uint16_t;
  using Timestamp = Time_t;

  using Endianness = SubmessageFlag;
  using FinalFlag = SubmessageFlag;
  using InlineQosFlag = SubmessageFlag;
  using DataFlag = SubmessageFlag;
  using KeyFlag = SubmessageFlag;
  using LivelinessFlag = SubmessageFlag;
  using MulticastFlag = SubmessageFlag;
  using InvalidateFlag = SubmessageFlag;


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
    pad_id = 0x1,
    acknack_id = 0x06,
    heartbeat_id = 0x07,
    gap_id = 0x08,
    info_ts_id = 0x09,
    info_src_id = 0x0c,
    info_reply_ip4_id = 0x0d,  // TODO need to propagate ipv4 vs. ipv6 to locator_list?
    info_dst_id = 0x0e,
    info_reply_id = 0x0f,  // TODO need to propagate ipv4 vs. ipv6 to locator_list?
    nack_frag_id = 0x12,
    heartbeat_frag_id = 0x13,
    data_id = 0x15,
    data_frag_id = 0x16
  };


  struct SubmessageHeader {
    BOOST_HANA_DEFINE_STRUCT(SubmessageHeader,
      (SubmessageKind, submessage_id),
      // (std::array<SubmessageFlag, 8>, flags),
      (std::bitset<8>, flags),
      (uint16_t, submessage_length)
    );

    template<typename T>
    static SubmessageHeader construct_submessage_header(const T & msg, uint16_t msg_size) {
      SubmessageHeader h;
      h.submessage_id = T::id;
      msg.produce_flags(h.flags);
      h.submessage_length = msg_size + sizeof(SubmessageHeader);
      return h;
    }
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

  template<typename SubmessageElement>
  struct Submessage {
    BOOST_HANA_DEFINE_STRUCT(Submessage,
      (SubmessageHeader, header),
      (SubmessageElement, element)
    );

    Submessage() {
      header.submessage_id = SubmessageElement::id;
    }

    Submessage(SubmessageElement && e) : element(e) {
      header.submessage_id = SubmessageElement::id;
      element.produce_flags(header.flags);
    }
  };

}


#endif  // CMBML__SUBMESSAGE__HPP_
