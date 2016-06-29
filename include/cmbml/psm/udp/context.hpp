#ifndef CMBML__PSM__UDP__CONTEXT_HPP_
#define CMBML__PSM__UDP__CONTEXT_HPP_

#include <cmbml/psm/udp/constants.hpp>
#include <cmbml/psm/udp/ports.hpp>
#include <cmbml/message/submessage.hpp>

#include <map>

namespace cmbml {
namespace udp {


struct InfoReplyIp4 {
  BOOST_HANA_DEFINE_STRUCT(InfoReplyIp4,
    (MulticastFlag, multicast_flag),
    (Locator_t, unicast_locator),
    (Locator_t, multicast_locator));
  static const SubmessageKind id = SubmessageKind::info_reply_ip4_id;
};

// TODO Express relationship to Locator_t.
struct LocatorUDPv4_t {
  uint64_t address = 0;
  uint64_t port = 0;

  constexpr LocatorUDPv4_t(uint64_t addr, uint64_t p) : address(addr), port(p) { }

  LocatorUDPv4_t(const Locator_t & locator) : port(locator.port) {
    const std::array<Octet, 4> addr = {locator.address[0], locator.address[1],
                                       locator.address[2], locator.address[3]};
    address = address_from_dot_notation(addr);
  }

  constexpr static uint64_t address_from_dot_notation(
      const std::array<Octet, 4> & adr)
  {
    return (((adr[0] * 256l + adr[1]) * 256l) + adr[2]) * 256l + adr[3];
  }

  void set_address_dot_notation(const std::array<Octet, 4> & adr) {
    address = address_from_dot_notation(adr);
  }
};

// Networking context.
// TODO formalize traits of "NetworkContext"
class Context {
public:
  static constexpr LocatorUDPv4_t default_multicast_locator = {
    default_spdp_multicast_port(cmbml_test_domain_id),
    LocatorUDPv4_t::address_from_dot_notation({239, 255, 0, 1})
  };
  static constexpr LocatorUDPv4_t invalid_locator = {0, 0};

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

  Context();

  // Settings
  // This is pretty big...
  // using default_resend_data_period = Duration_t<30, 0>;

  void unicast_send(const Locator_t & locator, const uint32_t * packet, size_t size);

  void multicast_send(const Locator_t & locator, const uint32_t * packet, size_t size);

  void add_unicast_receiver(const Locator_t & locator);
  void add_multicast_receiver(const Locator_t & locator);

private:

  static void socket_send(
    int socket, const Locator_t & locator, const uint32_t * packet, size_t size);

  uint32_t local_address;  // what's the best type?

  int unicast_send_socket = -1;
  int multicast_send_socket = -1;

  std::map<uint16_t, int> port_socket_map;
  // int unicast_recv_socket = -1;
  // int multicast_recv_socket = -1;
};

}
}


#endif  // CMBML__PSM__UDP__CONTEXT_HPP_
