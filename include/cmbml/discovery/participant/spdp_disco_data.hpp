#ifndef CMBML__SPDP_DISCO_DATA__HPP_
#define CMBML__SPDP_DISCO_DATA__HPP_

#include <bitset>

#include <cmbml/dds/qos.hpp>
// #include <cmbml/discovery/common.hpp>
#include <cmbml/structure/locator.hpp>
#include <cmbml/types.hpp>

namespace cmbml {
using BuiltinEndpointSet_t = std::bitset<32>;

// These definitions indicate which index gets set in the bitfield when the option is turned on
// it could be quite efficient to set Spdp configuration options at compile time,
// is there a use case for changing them dynamically?
// The first two indices should always be set, otherwise participants are not discoverable.
enum class BuiltinEndpointKind : uint32_t {
  participant_writer   = 0,
  participant_reader   = 1,
  publications_writer  = 2,
  publications_reader  = 3,
  subscriptions_writer = 4,
  subscriptions_reader = 5,
};

// "ParticipantBuiltinTopicData", "ParticipantProxy", and "SpdpDiscoveredParticipantData"
// Should this be put into structs?
struct SpdpDiscoData {
  BOOST_HANA_DEFINE_STRUCT(SpdpDiscoData,
    // (BuiltinTopicKey_t, key),
    (dds::UserDataQosPolicy, user_data),
    (ProtocolVersion_t, protocol_version),
    (GuidPrefix_t, guid_prefix),
    (VendorId_t, vendor_id),
    (bool, expects_inline_qos),
    (BuiltinEndpointSet_t, available_builtin_endpoints),
    (List<Locator_t>, metatraffic_unicast_locator_list),
    (List<Locator_t>, metatraffic_multicast_locator_list),
    (List<Locator_t>, default_unicast_locator_list),
    (List<Locator_t>, default_multicast_locator_list),
    (Count_t, manual_liveliness_count),
    (Duration_t, leaseDuration)
  );
  // TODO Default values

  // TODO Define bitfield operations on available_builtin_endpoints

  SpdpDiscoData() : leaseDuration({100, 0}) {}
};

}  // namespace cmbml

#endif  // CMBML__SPDP_DISCO_DATA__HPP_
