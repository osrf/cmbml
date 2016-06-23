#ifndef CMBML__PSM__UDP__CONTEXT_HPP_
#define CMBML__PSM__UDP__CONTEXT_HPP_

#include <cmbml/psm/udp/constants.hpp>
#include <cmbml/psm/udp/ports.hpp>

namespace cmbml {
namespace udp {


struct InfoReplyIp4 {
  BOOST_HANA_DEFINE_STRUCT(InfoReply,
    (MulticastFlag, multicast_flag),
    (Locator_t, unicast_locator),
    (Locator_t, multicast_locator));
  static const SubmessageKind id = SubmessageKind::info_reply_ip4_id;
};

struct LocatorUDPv4_t {
  uint32_t address;
  uint32_t port;

  constexpr static uint32_t address_from_dot_notation(
      const std::array<Octet, 4> & adr)
  {
    address = (((adr[0] * 256 + b) * 256) + c) * 256 + d;
  }

  void set_address_dot_notation(const std::array<Octet, 4> & adr) {
    address = address_from_dot_notation(adr);
  }

  static const invalid_locator = {0};
};

struct Context {
  static const LocatorUDPv4_t default_multicast_locator = {
    LOCATOR_KIND_UDPv4,
    default_spdp_multicast_port(cmbml_test_domain_id),
    address_from_dot_notation(239, 255, 0, 1)
  };

  static const EntityId_t participant_id        = {0x0, 0x00, 0x01, BuiltinEntity::participant};
  static const EntityId_t sedp_topic_writer_id  = {0x0, 0x00, 0x02, BuiltinEntity::writer_with_key};
  static const EntityId_t sedp_topic_reader_id  = {0x0, 0x00, 0x02, BuiltinEntity::reader_with_key};
  static const EntityId_t sedp_pub_writer_id    = {0x0, 0x00, 0x03, BuiltinEntity::writer_with_key};
  static const EntityId_t sedp_pub_reader_id    = {0x0, 0x00, 0x03, BuiltinEntity::reader_with_key};
  static const EntityId_t sedp_sub_writer_id    = {0x0, 0x00, 0x04, BuiltinEntity::writer_with_key};
  static const EntityId_t sedp_sub_reader_id    = {0x0, 0x00, 0x04, BuiltinEntity::reader_with_key};
  static const EntityId_t spdp_writer_id        = {0x0, 0x01, 0x00, BuiltinEntity::writer_with_key};
  static const EntityId_t spdp_reader_id        = {0x0, 0x01, 0x00, BuiltinEntity::reader_with_key};
  static const EntityId_t participant_writer_id = {0x0, 0x02, 0x00, BuiltinEntity::writer_with_key};
  static const EntityId_t participant_reader_id = {0x0, 0x02, 0x00, BuiltinEntity::reader_with_key};


  // Settings
  // This is pretty big...
  using default_resend_data_period = Duration_t<30, 0>;

};

}
}


#endif  // CMBML__PSM__UDP__CONTEXT_HPP_
