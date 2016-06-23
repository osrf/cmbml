#ifndef CMBML__DATA__HPP_
#define CMBML__DATA__HPP_

#include <boost/hana/define_struct.hpp>
#include <boost/hana/map.hpp>

#include <cmbml/structure/history.hpp>
#include <cmbml/types.hpp>

#include <cmbml/message/submessage.hpp>

namespace cmbml {

  struct AckNack : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(AckNack,
      (FinalFlag, final_flag),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumberSet, reader_sn_state),
      (Count_t, count));
    static const SubmessageKind id = SubmessageKind::acknack_id;
    virtual ~AckNack() {};
  };

  // TODO How to propagate Parameter type upwards
  struct Data : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(Data,
      (InlineQosFlag, expects_inline_qos),
      (DataFlag, has_data),
      (KeyFlag, has_key),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumberSet, writer_sn_state),
      (List<Parameter>, inline_qos),
      (SerializedData, payload)
    );
    static const SubmessageKind id = SubmessageKind::data_id;

    Data() {}
    virtual ~Data() {};

    // Construct a serialized message from a cache change.
    // Wooo
    Data(const CacheChange && change, bool inline_qos, bool key) :
      expects_inline_qos(inline_qos), has_data(!change.data.empty()),
      has_key(key), writer_id(change.writer_guid.entity_id), payload(change.data)
    {
      // TODO: writer_sn_state?
    }
    Data(const ChangeForReader && change, bool inline_qos, bool key) :
      expects_inline_qos(inline_qos), has_data(!change.data.empty()),
      has_key(key), writer_id(change.writer_guid.entity_id), payload(change.data)
    {
      // TODO: writer_sn_state?
    }
  };

  struct DataFrag : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(DataFrag,
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
    static const SubmessageKind id = SubmessageKind::data_frag_id;
    virtual ~DataFrag() {};
  };

  struct Gap : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(Gap,
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, gap_start),
      (SequenceNumberSet, gap_list));
    static const SubmessageKind id = SubmessageKind::gap_id;
    Gap() {}
    Gap(const EntityId_t & r_id, const EntityId_t w_id, const SequenceNumber_t & start)
      : reader_id(r_id), writer_id(w_id), gap_start(start)
    {}
    virtual ~Gap() {};
  };

  struct Heartbeat : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(Heartbeat,
      (FinalFlag, final_flag),
      (LivelinessFlag, liveliness_flag),
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, first_sn),
      (SequenceNumber_t, last_sn),
      (Count_t, count));

    static const SubmessageKind id = SubmessageKind::heartbeat_id;
    Heartbeat() {}
    Heartbeat(
      const GUID_t & writer_guid,
      const SequenceNumber_t & seq_num_min,
      const SequenceNumber_t & seq_num_max)
    {
      // TODO
    }
    virtual ~Heartbeat() {};
  };

  struct HeartbeatFrag : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(HeartbeatFrag,
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, writer_seq),
      (FragmentNumber_t, last_fragment_num),
      (Count_t, count));
    static const SubmessageKind id = SubmessageKind::heartbeat_frag_id;
    virtual ~HeartbeatFrag() {};
  };

  struct InfoDestination : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoDestination,
      (GuidPrefix_t, guid_prefix));
    static const SubmessageKind id = SubmessageKind::info_dst_id;
    virtual ~InfoDestination() {};
  };

  struct InfoReply : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoReply,
      (MulticastFlag, multicast_flag),
      (List<Locator_t>, unicast_locator_list),
      (List<Locator_t>, multicast_locator_list));
    static const SubmessageKind id = SubmessageKind::info_reply_id;
    virtual ~InfoReply() {};
  };

  struct InfoSource : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoSource,
      (ProtocolVersion_t, protocol_version),
      (VendorId_t, vendor_id),
      (GuidPrefix_t, guid_prefix));
    static const SubmessageKind id = SubmessageKind::info_src_id;
    virtual ~InfoSource() {};
  };

  struct InfoTimestamp : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoTimestamp,
      (InvalidateFlag, invalidate_flag),
      (Timestamp, timestamp));
    static const SubmessageKind id = SubmessageKind::info_ts_id;
    virtual ~InfoTimestamp() {};
  };

  struct NackFrag : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(NackFrag,
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, writer_seq),
      (FragmentNumberSet, fragment_number_state),
      (Count_t, count));
    static const SubmessageKind id = SubmessageKind::nack_frag_id;
    virtual ~NackFrag() {};
  };
}

#endif  // CMBML__DATA__HPP_
