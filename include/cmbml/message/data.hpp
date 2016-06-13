#ifndef CMBML__DATA__HPP_
#define CMBML__DATA__HPP_

#include <boost/hana/define_struct.hpp>

#include <cmbml/structure/history.hpp>
#include <cmbml/types.hpp>

#include <cmbml/message/submessage.hpp>

namespace cmbml {

  struct AckNack {
    BOOST_HANA_DEFINE_STRUCT(AckNack,
      (Endianness, endianness_flag),
      (FinalFlag, final_flag),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumberSet, reader_sn_state),
      (Count_t, count));
  };

  // TODO How to propagate Parameter type upwards
  struct Data {
    BOOST_HANA_DEFINE_STRUCT(Data,
      (Endianness, endianness_flag),
      (InlineQosFlag, expects_inline_qos),
      (DataFlag, has_data),
      (KeyFlag, has_key),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumberSet, writer_sn_state),
      (List<Parameter>, inline_qos),
      (SerializedData, payload)
    );

    Data() {}

    // Construct a serialized message from a cache change.
    Data(const CacheChange & change, bool expects_inline_qos) {
      // TODO
    }
  };

  struct DataFrag {
    BOOST_HANA_DEFINE_STRUCT(DataFrag,
      (Endianness, endianness_flag),
      (InlineQosFlag, expects_inline_qos),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, writer_seq),
      (FragmentNumber_t, fragment_num),
      (uint16_t, fragments_in_submessage),
      (uint32_t, data_size),
      (uint16_t, fragment_size),
      (List<Parameter>, inline_qos),
      (SerializedData, payload));
  };

  struct Gap {
    BOOST_HANA_DEFINE_STRUCT(Gap,
      (Endianness, endianness_flag),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, gap_start),
      (SequenceNumberSet, gap_list));
  };

  struct Heartbeat {
    BOOST_HANA_DEFINE_STRUCT(Heartbeat,
      (Endianness, endianness_flag),
      (FinalFlag, final_flag),
      (LivelinessFlag, liveliness_flag),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, first_sn),
      (SequenceNumber_t, last_sn),
      (Count_t, count));

    Heartbeat() {}
    Heartbeat(
      const GUID_t & writer_guid,
      const SequenceNumber_t & seq_num_min,
      const SequenceNumber_t & seq_num_max)
    {
      // TODO
    }
  };

  struct HeartbeatFrag {
    BOOST_HANA_DEFINE_STRUCT(HeartbeatFrag,
      (Endianness, endianness_flag),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, writer_seq),
      (FragmentNumber_t, last_fragment_num),
      (Count_t, count));
  };

  struct InfoDestination {
    BOOST_HANA_DEFINE_STRUCT(InfoDestination,
      (Endianness, endianness_flag),
      (GuidPrefix_t, guid_prefix));
  };

  struct InfoReply {
    BOOST_HANA_DEFINE_STRUCT(InfoReply,
      (Endianness, endianness_flag),
      (MulticastFlag, multicast_flag),
      (List<Locator_t>, unicast_locator_list),
      (List<Locator_t>, multicast_locator_list));
  };

  struct InfoSource {
    BOOST_HANA_DEFINE_STRUCT(InfoSource,
      (Endianness, endianness_flag),
      (ProtocolVersion_t, protocol_version),
      (VendorId_t, vendor_id),
      (GuidPrefix_t, guid_prefix));
  };

  struct InfoTimestamp {
    BOOST_HANA_DEFINE_STRUCT(InfoTimestamp,
      (Endianness, endianness_flag),
      (InvalidateFlag, invalidate_flag),
      (Timestamp, timestamp));
  };

  struct NackFrag {
    BOOST_HANA_DEFINE_STRUCT(NackFrag,
      (Endianness, endianness_flag),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, writer_seq),
      (FragmentNumberSet, fragment_number_state),
      (Count_t, count));
  };

}

#endif  // CMBML__DATA__HPP_
