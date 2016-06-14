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

    Data() {}

    // Construct a serialized message from a cache change.
    Data(const CacheChange & change, bool expects_inline_qos) {
      // TODO
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
  };

  struct Gap : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(Gap,
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, gap_start),
      (SequenceNumberSet, gap_list));
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

    Heartbeat() {}
    Heartbeat(
      const GUID_t & writer_guid,
      const SequenceNumber_t & seq_num_min,
      const SequenceNumber_t & seq_num_max)
    {
      // TODO
    }
  };

  struct HeartbeatFrag : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(HeartbeatFrag,
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, writer_seq),
      (FragmentNumber_t, last_fragment_num),
      (Count_t, count));
  };

  struct InfoDestination : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoDestination,
      (GuidPrefix_t, guid_prefix));
  };

  struct InfoReply : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoReply,
      (MulticastFlag, multicast_flag),
      (List<Locator_t>, unicast_locator_list),
      (List<Locator_t>, multicast_locator_list));
  };

  struct InfoSource : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoSource,
      (ProtocolVersion_t, protocol_version),
      (VendorId_t, vendor_id),
      (GuidPrefix_t, guid_prefix));
  };

  struct InfoTimestamp : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(InfoTimestamp,
      (InvalidateFlag, invalidate_flag),
      (Timestamp, timestamp));
  };

  struct NackFrag : SubmessageElement {
    BOOST_HANA_DEFINE_STRUCT(NackFrag,
      (EntityId_t, reader_id),
      (EntityId_t, writer_id),
      (SequenceNumber_t, writer_seq),
      (FragmentNumberSet, fragment_number_state),
      (Count_t, count));
  };


  // This map really isn't that useful unless the submessage ID is going to be a constexpr.
  namespace hana = boost::hana;
  constexpr auto subelement_type_id_map = boost::hana::make_map(
    // hana::make_pair(SubmessageKind::pad, hana::type_c<Pad>),
    hana::make_pair(hana::uint_c<SubmessageKind::acknack>, hana::type_c<AckNack>),
    hana::make_pair(hana::uint_c<SubmessageKind::heartbeat>, hana::type_c<Heartbeat>),
    hana::make_pair(hana::uint_c<SubmessageKind::gap>, hana::type_c<Gap>),
    hana::make_pair(hana::uint_c<SubmessageKind::info_ts>, hana::type_c<InfoTimestamp>),
    hana::make_pair(hana::uint_c<SubmessageKind::info_src>, hana::type_c<InfoSource>),
    hana::make_pair(hana::uint_c<SubmessageKind::info_reply_ip4>, hana::type_c<InfoReply>),
    hana::make_pair(hana::uint_c<SubmessageKind::info_reply>, hana::type_c<InfoReply>),
    hana::make_pair(hana::uint_c<SubmessageKind::info_dst>, hana::type_c<InfoDestination>),
    hana::make_pair(hana::uint_c<SubmessageKind::nack_frag>, hana::type_c<NackFrag>),
    hana::make_pair(hana::uint_c<SubmessageKind::heartbeat_frag>, hana::type_c<HeartbeatFrag>),
    hana::make_pair(hana::uint_c<SubmessageKind::data>, hana::type_c<Data>),
    hana::make_pair(hana::uint_c<SubmessageKind::data_frag>, hana::type_c<DataFrag>)
  );

}

#endif  // CMBML__DATA__HPP_
