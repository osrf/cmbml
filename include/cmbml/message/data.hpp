#ifndef CMBML__DATA__HPP_
#define CMBML__DATA__HPP_

#include <cmbml/structure/history.hpp>
#include <cmbml/types.hpp>

#include <cmbml/message/submessage.hpp>

namespace cmbml {

  struct AckNack {
    Endianness endianness_flag;
    FinalFlag final_flag;
    EntityId_t reader_id;
    EntityId_t writer_id;
    SequenceNumberSet reader_sn_state;
    Count_t count;
  };

  struct Data {
    Endianness endianness_flag;
    InlineQosFlag expects_inline_qos;
    DataFlag has_data;
    KeyFlag has_key;
    EntityId_t reader_id;
    EntityId_t writer_id;
    SequenceNumberSet writer_sn_state;
    // isn't there a nicer way to store a list of parameters that share a common template?
    List<ParameterIt> inline_qos;
    SerializedData payload;

    // Construct a serialized message from a cache change.
    Data(const CacheChange & change, bool expects_inline_qos) {
      // TODO
    }
  };

  struct DataFrag {
    Endianness endianness_flag;
    InlineQosFlag expects_inline_qos;
    EntityId_t reader_id;
    EntityId_t writer_id;
    SequenceNumber_t writer_seq;
    FragmentNumber_t fragment_num;
    uint16_t fragments_in_submessage;
    uint32_t data_size;
    uint16_t fragment_size;
    List<ParameterIt> inline_qos;
    SerializedData payload;
  };

  struct Gap {
    Endianness endianness_flag;
    EntityId_t reader_id;
    EntityId_t writer_id;
    SequenceNumber_t gap_start;
    SequenceNumberSet gap_list;
  };

  struct Heartbeat {
    Endianness endianness_flag;
    FinalFlag final_flag;
    LivelinessFlag liveliness_flag;
    EntityId_t reader_id;
    EntityId_t writer_id;
    SequenceNumber_t first_sn;
    SequenceNumber_t last_sn;
    Count_t count;

    Heartbeat(
      const GUID_t & writer_guid,
      const SequenceNumber_t & seq_num_min,
      const SequenceNumber_t & seq_num_max)
    {
      // TODO
    }
  };

  struct HeartbeatFrag {
    Endianness endianness_flag;
    EntityId_t reader_id;
    EntityId_t writer_id;
    SequenceNumber_t writer_seq;
    FragmentNumber_t last_fragment_num;
    Count_t count;
  };

  struct InfoDestination {
    Endianness endianness_flag;
    GuidPrefix_t guid_prefix;
  };

  struct InfoReply {
    Endianness endianness_flag;
    MulticastFlag multicast_flag;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
  };

  struct InfoSource {
    Endianness endianness_flag;
    ProtocolVersion_t protocol_version;
    VendorId_t vendor_id;
    GuidPrefix_t guid_prefix;
  };

  struct InfoTimestamp {
    Endianness endianness_flag;
    InvalidateFlag invalidate_flag;
    Timestamp timestamp;
  };

  struct NackFrag {
    Endianness endianness_flag;
    EntityId_t reader_id;
    EntityId_t writer_id;
    SequenceNumber_t writer_seq;
    FragmentNumberSet fragment_number_state;
    Count_t count;
  };

}

#endif  // CMBML__DATA__HPP_
