#ifndef CMBML__STRUCTURE__PARTICIPANT__HPP_
#define CMBML__STRUCTURE__PARTICIPANT__HPP_

#include <cmbml/discovery/participant/spdp_disco_data.hpp>
#include <cmbml/message/header.hpp>
#include <cmbml/message/submessage.hpp>
#include <cmbml/serialization/serialize_cdr.hpp>
#include <cmbml/structure/locator.hpp>
#include <cmbml/types.hpp>

// TODO this COULD be templated on the discovery protocol type.

namespace cmbml {

  static constexpr EntityId_t participant_id = {0x0, 0x0, 0x1, EntityKind::participant};

  static constexpr EntityId_t sedp_topic_writer_id =
    {0x0, 0x0, 0x2, EntityKind::builtin_writer_with_key};

  static constexpr EntityId_t sedp_topic_reader_id =
    {0x0, 0x0, 0x2, EntityKind::builtin_reader_with_key};

  static constexpr EntityId_t sedp_pub_writer_id =
    {0x0, 0x0, 0x3, EntityKind::builtin_writer_with_key};

  static constexpr EntityId_t sedp_pub_reader_id =
    {0x0, 0x0, 0x3, EntityKind::builtin_reader_with_key};

  static constexpr EntityId_t sedp_sub_writer_id =
    {0x0, 0x0, 0x4, EntityKind::builtin_writer_with_key};

  static constexpr EntityId_t sedp_sub_reader_id =
    {0x0, 0x0, 0x4, EntityKind::builtin_reader_with_key};

  static constexpr EntityId_t spdp_writer_id =
    {0x0, 0x1, 0x0, EntityKind::builtin_writer_with_key};

  static constexpr EntityId_t spdp_reader_id =
    {0x0, 0x1, 0x0, EntityKind::builtin_reader_with_key};

  static constexpr EntityId_t participant_writer_id =
    {0x0, 0x2, 0x0, EntityKind::builtin_writer_with_key};

  static constexpr EntityId_t participant_reader_id =
    {0x0, 0x2, 0x0, EntityKind::builtin_reader_with_key};


  struct Participant : Entity {
    Participant(SpdpDiscoData & data) :
      protocol_version(data.protocol_version),
      vendor_id(data.vendor_id),
      default_unicast_locator_list(data.default_unicast_locator_list),
      default_multicast_locator_list(data.default_multicast_locator_list),
      Entity({data.guid_prefix, participant_id})
    {
    }

    Participant(GuidPrefix_t && prefix, List<Locator_t> && multicast_list) :
      default_multicast_locator_list(multicast_list),
      protocol_version(rtps_protocol_version),
      vendor_id(cmbml_vendor_id),
      Entity({prefix, participant_id})
    {
    }

    // a participant "contains" Endpoints
    // Lists of endpoints. 
    List<Locator_t> default_unicast_locator_list;
    List<Locator_t> default_multicast_locator_list;
    ProtocolVersion_t protocol_version = rtps_protocol_version;
    VendorId_t vendor_id = cmbml_vendor_id;

    template<typename EntityKind>
    EntityId_t assign_next_entity_id() {
      EntityId_t ret;
      ret.entity_key = EntityId_t::convert(++current_entity_id);
      ret.entity_kind = EntityKind::entity_kind;
      return ret;
    }

    Header construct_message_header() const {
      return Header{rtps_protocol_id, protocol_version, vendor_id, guid.prefix};
    }

    template<typename T>
    Packet<> serialize_with_header(T && msg) const {
      size_t packet_size = get_packet_size(msg);
      Packet<> packet((sizeof(Header) + sizeof(SubmessageHeader) + packet_size)/4);
      Header h = construct_message_header();
      SubmessageHeader sub_h = SubmessageHeader::construct_submessage_header(msg, packet_size);

      size_t index = 0;
      serialize(h, packet, index);
      serialize(sub_h, packet, index);
      serialize(msg, packet, index);
      return packet;
    }

    SpdpDiscoData create_discovery_data() const {
      SpdpDiscoData ret;
      ret.protocol_version = protocol_version;
      ret.guid_prefix = guid.prefix;
      ret.vendor_id = vendor_id;
      ret.default_unicast_locator_list = default_unicast_locator_list;
      ret.default_multicast_locator_list = default_multicast_locator_list;
      return ret;
    }

  private:
    uint32_t current_entity_id = 0;
  };


}  // namespace cmbml

#endif  // CMBML__STRUCTURE__PARTICIPANT__HPP_
