#ifndef CMBML__STRUCTURE__PARTICIPANT__HPP_
#define CMBML__STRUCTURE__PARTICIPANT__HPP_

#include <cmbml/discovery/participant/spdp_disco_data.hpp>
#include <cmbml/message/header.hpp>
#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/types.hpp>

// TODO this COULD be templated on the discovery protocol type.

namespace cmbml {

struct Participant : Entity {
  explicit Participant(SpdpDiscoData & data) :
    default_unicast_locator_list(data.default_unicast_locator_list),
    default_multicast_locator_list(data.default_multicast_locator_list),
    protocol_version(data.protocol_version),
    vendor_id(data.vendor_id),
    Entity({data.guid_prefix, 0, 0, 0, EntityKind::participant})
  {
  }

  Participant(GuidPrefix_t & prefix, List<Locator_t> & multicast_list) :
    default_multicast_locator_list(multicast_list),
    protocol_version(rtps_protocol_version),
    vendor_id(cmbml_vendor_id),
    Entity({prefix, 0, 0, 0, EntityKind::participant})
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
    Packet<> packet(sizeof(Header) + packet_size);
    Header h = construct_message_header();

    size_t index = 0;
    serialize(h, packet, index);
    serialize(msg, packet, index);
    return packet;
  }

private:
  uint32_t current_entity_id = 0;
};


}  // namespace cmbml

#endif  // CMBML__STRUCTURE__PARTICIPANT__HPP_
