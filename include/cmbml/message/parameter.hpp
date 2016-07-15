#ifndef CMBML__PARAMETER__HPP_
#define CMBML__PARAMETER__HPP_

#include <cmbml/types.hpp>

namespace cmbml {

  // TODO Type map? need sketch of ParameterID Serialization.
  enum struct ParameterId_t : uint16_t {
    sentinel = 0x1,
    user_data = 0x2c,
    topic_name = 0x5,
    type_name = 0x7,
    group_data = 0x2d,
    topic_data = 0x2e,
    durability = 0x1d,
    durability_service = 0x1e,
    deadline = 0x23,
    latency_budget = 0x27,
    liveliness = 0x1b,
    reliability = 0x1a,
    lifespan = 0x2b,
    destination_order = 0x25,
    history = 0x40,
    resource_limits = 0x41,
    ownership = 0x1f,
    ownership_strength = 0x6,
    presentation = 0x21,
    partition = 0x29,
    time_based_filter = 0x4,
    transport_priority = 0x49,
    protocol_version = 0x15,
    vendor_id = 0x16,
    unicast_locator = 0x2f,
    multicast_locator = 0x30,
    multicast_ipaddress = 0x11,
    default_unicast_locator = 0x31,
    default_multicast_locator = 0x48,
    metatraffic_unicast_locator = 0x32,
    metatraffic_multicast_locator = 0x33,
    default_unicast_ipaddress = 0xc,
    default_unicast_port = 0xe,
    metatraffic_unicast_ipaddress = 0x45,
    metatraffic_unicast_port = 0xd,
    metatraffic_multicast_port = 0x46,
    expects_inline_qos = 0x43,
    participant_manual_liveliness_count = 0x34,
    participant_builtin_endpoints = 0x44,
    participant_lease_duration = 0x2,
    content_filter_property = 0x35,
    participant_guid = 0x50,
    participant_entityid = 0x51,
    group_guid = 0x52,
    group_entityid = 0x53,
    builtin_endpoint_set = 0x58,
    property_list = 0x59,
    type_max_size_serialized = 0x60,
    entity_name = 0x62,
    key_hash = 0x70,
    status_info = 0x71
  };

  struct Parameter {
    BOOST_HANA_DEFINE_STRUCT(Parameter,
    (ParameterId_t, id),
    (uint16_t, length),
    (List<Octet>, value));
  };

}

#endif  // CMBML__PARAMETER__HPP_
