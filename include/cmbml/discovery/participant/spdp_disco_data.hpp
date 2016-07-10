#ifndef CMBML__SPDP_DISCO_DATA__HPP_
#define CMBML__SPDP_DISCO_DATA__HPP_

#include <bitset>

#include <cmbml/dds/qos.hpp>
// #include <cmbml/discovery/common.hpp>
#include <cmbml/structure/locator.hpp>
#include <cmbml/types.hpp>

namespace cmbml {
using BuiltinEndpointSet_t = std::bitset<8>;

enum class BuiltinEndpointKind : uint8_t {
  publications_reader,
  publications_writer,
  subscriptions_reader,
  subscriptions_writer,
  topic_reader,
  topic_writer
};

// "ParticipantBuiltinTopicData", "ParticipantProxy", and "SpdpDiscoveredParticipantData"
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
