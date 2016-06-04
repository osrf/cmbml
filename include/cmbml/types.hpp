/* Structs defined by the RTPS spec
 * 
 */

#ifndef CMBML__TYPES__HPP_
#define CMBML__TYPES__HPP_

#include <array>
#include <chrono>  // Oh boy, I hope chrono isn't too big
#include <cstdint>
#include <vector>

namespace cmbml {

  enum class ReliabilityKind_t {
    best_effort,
    reliable
  };
  enum class TopicKind_t {
    no_key,
    with_key
  };

  using Octet = uint8_t;
  // 12 octets
  using GuidPrefix_t = std::array<Octet, 12>;
  // 4 octets
  using EntityId_t = std::array<Octet, 4>;
  using VendorId_t = std::array<Octet, 2>;
  static const VendorId_t cmbml_vendor_id = {0xf0, 0x9f};

  // TODO: Figure out a better embedded-friendly sequence implementation.
  // are we going to pass around an allocator std vector?
  template<typename T, typename Allocator = std::allocator<T>>
  using List = std::vector<T, Allocator>;

  //using Duration_t = std::chrono::nanoseconds;
  struct Duration_t {
    uint32_t sec;
    uint32_t nsec;
  };

  // 16-byte (128 bit) GUID
  struct GUID_t {
    // Uniquely identifies the Participant within the Domain.
    GuidPrefix_t prefix;
    // Uniquely identifies the Entity within the Participant.
    EntityId_t entity_id;
  };

  struct Entity {
    // Globally and uniquely identifies the
    // RTPS Entity within the DDS domain.
    GUID_t guid;
  };

  struct Locator_t {
    int32_t kind;
    uint32_t port;
    std::array<Octet, 16> address;
  };

  struct ProtocolVersion_t {
    Octet major;
    Octet minor;
  };
  // lower protocol versions are not supported
  static const ProtocolVersion_t rtps_protocol_version = {2, 2};

  struct Participant : Entity {
    // a participant "contains" Endpoints
    // Lists of endpoints. 
    List<Locator_t> default_unicast_locator_list;
    List<Locator_t> default_multicast_locator_list;
    //static constexpr ProtocolVersion_t protocol_version = rtps_protocol_version;
    //static constexpr VendorId_t vendor_id = cmbml_vendor_id;
  };


  template<ReliabilityKind_t reliabilityLevel, TopicKind_t topicKind>
  struct EndpointParams {
    static const ReliabilityKind_t reliability_level = reliabilityLevel;
    static const TopicKind_t topic_kind = topicKind;
  };

  // TODO Template error checking
  template<typename EndpointParams>
  struct Endpoint : Entity {
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
    static const ReliabilityKind_t reliability_level = EndpointParams::reliability_level;
    static const TopicKind_t topic_kind = EndpointParams::topic_kind;
  };

}

#endif  // CMBML__TYPES__HPP_
