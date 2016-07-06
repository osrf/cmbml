#ifndef CMBML__STRUCTURE__PARTICIPANT__HPP_
#define CMBML__STRUCTURE__PARTICIPANT__HPP_

#include <cmbml/types.hpp>
#include <cmbml/discovery/participant/spdp_disco_data.hpp>

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
    Entity({data.guid_prefix, 0, 0, 0, EntityKind::participant})
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

private:
  uint32_t current_entity_id = 0;
};


}  // namespace cmbml

#endif  // CMBML__STRUCTURE__PARTICIPANT__HPP_
