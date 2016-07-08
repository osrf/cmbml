#ifndef CMBML__SPDP_DISCO_DATA__HPP_
#define CMBML__SPDP_DISCO_DATA__HPP_

#include <bitset>

#include <cmbml/structure/locator.hpp>
#include <cmbml/types.hpp>

namespace cmbml {
using BuiltinEndpointSet_t = std::bitset<6>;

enum class BuiltinEndpointKind : uint8_t {
  publications_reader,
  publications_writer,
  subscriptions_reader,
  subscriptions_writer,
  topic_reader,
  topic_writer
};

// Make serializable
struct SpdpDiscoData {
  ProtocolVersion_t protocol_version;
  GuidPrefix_t guid_prefix;
  VendorId_t vendor_id;
  bool expects_inline_qos = false;
  List<Locator_t> metatraffic_unicast_locator_list;
  List<Locator_t> metatraffic_multicast_locator_list;
  List<Locator_t> default_unicast_locator_list;
  List<Locator_t> default_multicast_locator_list;

  // This should get aligned to a byte in the message!
  // TODO This is basically a bitfield stuffed into a byte--define operations
  BuiltinEndpointSet_t available_builtin_endpoints;
  Duration_t leaseDuration{100, 0};
  Count_t manual_liveliness_count;
};

}  // namespace cmbml

#endif  // CMBML__SPDP_DISCO_DATA__HPP_
