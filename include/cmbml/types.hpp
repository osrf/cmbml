/* Structs defined by the RTPS spec
 * 
 */

#ifndef CMBML__TYPES__HPP_
#define CMBML__TYPES__HPP_

#include <boost/hana/define_struct.hpp>
#include <chrono>

#include <array>
#include <cstdint>
#include <vector>

namespace cmbml {
  // TODO this should be selectable at compile time
  static const uint32_t cmbml_test_domain_id = 1337;

  enum class ReliabilityKind_t {
    best_effort = 1,
    reliable = 3
  };
  enum class TopicKind_t {
    no_key = 1,
    with_key = 2
  };

  using Octet = uint8_t;
  // 12 octets
  using GuidPrefix_t = std::array<Octet, 12>;
  static const GuidPrefix_t guid_prefix_unknown = {0};
  // 4 octets
  using EntityId_t = std::array<Octet, 4>;
  static const EntityId_t entity_id_unknown = {0x00, 0x00, 0x00, 0x00};
  using VendorId_t = std::array<Octet, 2>;
  static const VendorId_t cmbml_vendor_id = {0xf0, 0x9f};
  static const VendorId_t vendor_id_unknown = {0x0, 0x0};

  using ProtocolId_t = std::array<char, 4>;
  static const ProtocolId_t protocol_id = {'R', 'T', 'P', 'S'};

  // TODO: Figure out a better embedded-friendly sequence implementation.
  // are we going to pass around an allocator std vector?
  template<typename T, typename Allocator = std::allocator<T>>
  using List = std::vector<T, Allocator>;

  //using Duration_t = std::chrono::nanoseconds;
  struct Duration_t {
    constexpr Duration_t(uint32_t s, uint32_t ns) : sec(s), nsec(ns) {}

    uint32_t sec;
    uint32_t nsec;
    std::chrono::nanoseconds to_ns() const {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(sec)) +
        std::chrono::nanoseconds(nsec);
    };
  };

  // Interesting, a fixed-point time representation (IETF RFC 1305)
  struct Time_t {
    BOOST_HANA_DEFINE_STRUCT(Time_t,
    (int32_t, seconds),
    (uint32_t, fraction));  // represents sec/2^32
    Time_t() : seconds(0), fraction(0) {};

    constexpr Time_t(int32_t sec, uint32_t frac) : seconds(sec), fraction(frac) {};
  };

  // TODO These constants should have better names (namespaced, or different case)
  constexpr static Time_t time_zero = Time_t(0, 0);
  // woah. Actually, time_t should be 64-bit ints then?
  constexpr static Time_t time_invalid = Time_t(-1, 0xffffffff);
  constexpr static Time_t time_infinite = Time_t(0x7fffffff, 0xffffffff);

  struct SequenceNumber_t {
    BOOST_HANA_DEFINE_STRUCT(SequenceNumber_t,
    (int32_t, high),
    (uint32_t, low));

    constexpr uint64_t value() const {
      return high*multiplicand + low;
    }

    constexpr static bool less_than(const SequenceNumber_t & a, const SequenceNumber_t & b) {
      return a.value() < b.value();
    }
    bool operator<(const SequenceNumber_t & a) {
      return this->value() < a.value();
    }
    bool operator>=(const SequenceNumber_t & a) const {
      return this->value() >= a.value();
    }
    bool operator<=(const SequenceNumber_t & a) const {
      return this->value() <= a.value();
    }
    bool operator>(const SequenceNumber_t & a) const {
      return this->value() > a.value();
    }
    constexpr static bool equal(const SequenceNumber_t & a, const SequenceNumber_t & b) {
      return a.value() == b.value();
    }
    template<typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    constexpr SequenceNumber_t operator+(const T i) const {
      // TODO Overflow
      return SequenceNumber_t({this->high, this->low + static_cast<uint32_t>(i)});
    }

    template<typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    constexpr SequenceNumber_t operator-(const T i) const {
      // TODO Overflow
      return SequenceNumber_t({this->high, this->low - static_cast<uint32_t>(i)});
    }
    static const uint64_t multiplicand = static_cast<uint64_t>(1) << 32;
  };


  constexpr static SequenceNumber_t minimum_sequence_number = {INT32_MAX, UINT32_MAX};
  constexpr static SequenceNumber_t maximum_sequence_number = {INT32_MIN, 0};

  struct SequenceNumberCompare {
    constexpr bool operator()(const SequenceNumber_t & a, const SequenceNumber_t & b) {
      return SequenceNumber_t::less_than(a, b);
    }
  };

  // 16-byte (128 bit) GUID
  struct GUID_t {
    // TODO Prefix always sets the first 2 bytes to vendor ID
    // prefix: Uniquely identifies the Participant within the Domain.
    // entity_id: Uniquely identifies the Entity within the Participant.
    BOOST_HANA_DEFINE_STRUCT(GUID_t,
      (GuidPrefix_t, prefix),
      (EntityId_t, entity_id)
    );

    bool operator==(const GUID_t & b) const {
      for (size_t i = 0; i < entity_id.size(); ++i) {
        if (entity_id[i] != b.entity_id[i]) {
          return false;
        }
      }
      for (size_t i = 0; i < prefix.size(); ++i) {
        if (prefix[i] != b.prefix[i]) {
          return false;
        }
      }
      return true;
    }
  };

  // TODO: collisions with endpoints outside of participant
  struct GUIDCompare {
    constexpr bool operator()(const GUID_t & a, const GUID_t & b) const {
      bool less = true;
      for (size_t i = 0; i < 4; ++i) {
        if (a.entity_id[i] > b.entity_id[i]) {
          return false;
        }
      }
      for (size_t i = 0; i < 12; ++i) {
        if (a.prefix[i] > b.prefix[i]) {
          return false;
        }
      }

      return true;
    }
  };

  struct Entity {
    // Globally and uniquely identifies the
    // RTPS Entity within the DDS domain.
    GUID_t guid;
  };

  struct Locator_t {
    BOOST_HANA_DEFINE_STRUCT(Locator_t,
    (int32_t, kind),
    (uint32_t, port),
    (std::array<Octet, 16>, address));
  };

  struct ProtocolVersion_t {
    BOOST_HANA_DEFINE_STRUCT(ProtocolVersion_t,
    (Octet, major),
    (Octet, minor));
  };
  // lower protocol versions are not supported
  static const ProtocolVersion_t rtps_protocol_version = {2, 2};

  struct Participant : Entity {
    // a participant "contains" Endpoints
    // Lists of endpoints. 
    List<Locator_t> default_unicast_locator_list;
    List<Locator_t> default_multicast_locator_list;
    uint32_t participant_id;
    //static constexpr ProtocolVersion_t protocol_version = rtps_protocol_version;
    //static constexpr VendorId_t vendor_id = cmbml_vendor_id;
  };


  template<ReliabilityKind_t reliabilityLevel, TopicKind_t topicKind>
  struct EndpointParams {
    static const ReliabilityKind_t reliability_level = reliabilityLevel;
    static const TopicKind_t topic_kind = topicKind;
  };

  // TODO Template error checking
  // On initialization, initialize Transport-specific data with this Endpoint.
  template<typename EndpointParams>
  struct Endpoint : Entity {
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
    static const ReliabilityKind_t reliability_level = EndpointParams::reliability_level;
    static const TopicKind_t topic_kind = EndpointParams::topic_kind;
  };

}

#endif  // CMBML__TYPES__HPP_
