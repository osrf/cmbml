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
  static const GuidPrefix_t guid_prefix_unknown = {{0}};
  using VendorId_t = std::array<Octet, 2>;
  static const VendorId_t cmbml_vendor_id = {{0xf0, 0x9f}};
  static const VendorId_t vendor_id_unknown = {{0x0, 0x0}};

  using ProtocolId_t = std::array<char, 4>;
  static const ProtocolId_t rtps_protocol_id = {{'R', 'T', 'P', 'S'}};

  // TODO: Figure out a better embedded-friendly sequence implementation.
  // are we going to pass around an allocator std vector?
  template<typename T, typename Allocator = std::allocator<T>>
  using List = std::vector<T, Allocator>;
  // TODO: Better embedded-friendly string class
  using String = std::string;

  using Count_t = uint32_t;

  struct Duration_t {
    BOOST_HANA_DEFINE_STRUCT(Duration_t,
      (uint32_t, sec),
      (uint32_t, nsec)
    );

    constexpr Duration_t(uint32_t s, uint32_t ns) : sec(s), nsec(ns) {}
    constexpr Duration_t() : sec(0), nsec(0) {}

    std::chrono::nanoseconds to_ns() const {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(sec)) +
        std::chrono::nanoseconds(nsec);
    };
  };

  template<uint32_t Sec, uint32_t Nsec>
  struct DurationT {
    static const uint32_t sec = Sec;
    static const uint32_t nsec = Nsec;

    static constexpr std::chrono::nanoseconds to_ns()
    {
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

  enum struct EntityKind : uint8_t {
    unknown = 0x00,
    user_writer_with_key = 0x02,
    user_writer_no_key = 0x03,
    user_reader_no_key = 0x04,
    user_reader_with_key = 0x07,
    participant = 0xc1,
    builtin_writer_with_key = 0xc2,
    builtin_writer_no_key = 0xc3,
    builtin_reader_no_key = 0xc4,
    builtin_reader_with_key = 0xc7
  };


  struct EntityId_t {
    BOOST_HANA_DEFINE_STRUCT(EntityId_t,
      (std::array<Octet, 3>, entity_key),
      (EntityKind, entity_kind)
    );

    bool operator==(const EntityId_t & e) const {
      if (e.entity_kind != entity_kind) {
        return false;
      }
      for (size_t i = 0; i < 3; ++i) {
        if (e.entity_key[i] != entity_key[i]) {
          return false;
        }
      }
      return true;
    }
    bool operator!=(const EntityId_t & e) const {
      return !operator==(e);
    }

    // internal representation is big-endian by default
    static uint32_t convert(const std::array<Octet, 3> & id) {
      return (id[0]*256 + id[1]) * 256 + id[2];
    }

    // consider unit testing this
    static std::array<Octet, 3> convert(const uint32_t id) {
      std::array<Octet, 3> ret;
      uint32_t mask = UINT32_MAX >> 16;
      ret[2] = id & mask;
      mask = (UINT32_MAX >> 16) << 8;
      ret[1] = id & mask;
      mask = UINT32_MAX << 16;
      ret[0] = id & mask;
      return ret;
    }
  };

  static const EntityId_t entity_id_unknown = {{{0x00, 0x00, 0x00}}, EntityKind::unknown};

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
      if (entity_id != b.entity_id) {
        return false;
      }
      for (size_t i = 0; i < prefix.size(); ++i) {
        if (prefix[i] != b.prefix[i]) {
          return false;
        }
      }
      return true;
    }

    bool operator<(const GUID_t & a) const {
      for (size_t i = 0; i < 3; ++i) {
        if (this->entity_id.entity_kind > a.entity_id.entity_kind) {
          return false;
        }
        if (this->entity_id.entity_key[i] > a.entity_id.entity_key[i]) {
          return false;
        }
      }
      for (size_t i = 0; i < 12; ++i) {
        if (this->prefix[i] > a.prefix[i]) {
          return false;
        }
      }
      return true;
    }
  };

  // TODO: collisions with endpoints outside of participant
  struct GUIDCompare {
    constexpr bool operator()(const GUID_t & a, const GUID_t & b) const {
      return a < b;
    }
  };

  struct Entity {
    Entity(GUID_t g) : guid(g) {}
    Entity() {}
    // Globally and uniquely identifies the
    // RTPS Entity within the DDS domain.
    GUID_t guid;
  };

  struct ProtocolVersion_t {
    BOOST_HANA_DEFINE_STRUCT(ProtocolVersion_t,
    (Octet, major),
    (Octet, minor));
  };
  // lower protocol versions are not supported
  static const ProtocolVersion_t rtps_protocol_version = {2, 2};
}

#endif  // CMBML__TYPES__HPP_
