#ifndef CMBML__PSM__UDP__CONTEXT_HPP_
#define CMBML__PSM__UDP__CONTEXT_HPP_

#include <cmbml/psm/udp/constants.hpp>
#include <cmbml/psm/udp/ports.hpp>

namespace cmbml {
namespace udp {


struct InfoReplyIp4 {
  BOOST_HANA_DEFINE_STRUCT(InfoReplyIp4,
    (MulticastFlag, multicast_flag),
    (Locator_t, unicast_locator),
    (Locator_t, multicast_locator));
  static const SubmessageKind id = SubmessageKind::info_reply_ip4_id;
};

struct LocatorUDPv4_t {
  uint64_t address;
  uint64_t port;

  constexpr static uint64_t address_from_dot_notation(
      const std::array<Octet, 4> & adr)
  {
    return (((adr[0] * 256l + adr[1]) * 256l) + adr[2]) * 256l + adr[3];
  }

  void set_address_dot_notation(const std::array<Octet, 4> & adr) {
    address = address_from_dot_notation(adr);
  }

};

struct Context {
  static constexpr LocatorUDPv4_t default_multicast_locator = {
    default_spdp_multicast_port(cmbml_test_domain_id),
    LocatorUDPv4_t::address_from_dot_notation({239, 255, 0, 1})
  };
  static constexpr LocatorUDPv4_t invalid_locator = {0};

  static constexpr EntityId_t participant_id =
    {0x0, 0x0, 0x1, static_cast<uint8_t>(BuiltinEntity::participant)};
  static constexpr EntityId_t sedp_topic_writer_id =
    {0x0, 0x0, 0x2, static_cast<uint8_t>(BuiltinEntity::writer_with_key)};
  static constexpr EntityId_t sedp_topic_reader_id =
    {0x0, 0x0, 0x2, static_cast<uint8_t>(BuiltinEntity::reader_with_key)};
  static constexpr EntityId_t sedp_pub_writer_id =
    {0x0, 0x0, 0x3, static_cast<uint8_t>(BuiltinEntity::writer_with_key)};
  static constexpr EntityId_t sedp_pub_reader_id =
    {0x0, 0x0, 0x3, static_cast<uint8_t>(BuiltinEntity::reader_with_key)};
  static constexpr EntityId_t sedp_sub_writer_id =
    {0x0, 0x0, 0x4, static_cast<uint8_t>(BuiltinEntity::writer_with_key)};
  static constexpr EntityId_t sedp_sub_reader_id =
    {0x0, 0x0, 0x4, static_cast<uint8_t>(BuiltinEntity::reader_with_key)};
  static constexpr EntityId_t spdp_writer_id =
    {0x0, 0x1, 0x0, static_cast<uint8_t>(BuiltinEntity::writer_with_key)};
  static constexpr EntityId_t spdp_reader_id =
    {0x0, 0x1, 0x0, static_cast<uint8_t>(BuiltinEntity::reader_with_key)};
  static constexpr EntityId_t participant_writer_id =
    {0x0, 0x2, 0x0, static_cast<uint8_t>(BuiltinEntity::writer_with_key)};
  static constexpr EntityId_t participant_reader_id =
    {0x0, 0x2, 0x0, static_cast<uint8_t>(BuiltinEntity::reader_with_key)};


  // Settings
  // This is pretty big...
  // using default_resend_data_period = Duration_t<30, 0>;

};

}
}


#endif  // CMBML__PSM__UDP__CONTEXT_HPP_
