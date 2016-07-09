#ifndef CMBML__PSM__UDP__CONTEXT_HPP_
#define CMBML__PSM__UDP__CONTEXT_HPP_

#include <cmbml/cdr/common.hpp>
#include <cmbml/psm/udp/constants.hpp>
#include <cmbml/psm/udp/ports.hpp>
#include <cmbml/message/submessage.hpp>
#include <cmbml/structure/locator.hpp>

#include <sys/socket.h>

#include <map>

namespace cmbml {
namespace udp {

// UDP Max fragment size
#define CMBML__MAX_FRAGMENT_SIZE 65507

struct InfoReplyIp4 {
  BOOST_HANA_DEFINE_STRUCT(InfoReplyIp4,
    (Locator_t, unicast_locator),
    (Locator_t, multicast_locator));
  static const SubmessageKind id = SubmessageKind::info_reply_ip4_id;
  Endianness endianness;
  MulticastFlag multicast_flag;

  void assign_flags(const std::bitset<8> & flags) {
    endianness = flags[7];
    multicast_flag = flags[6];
  }
  void produce_flags(std::bitset<8> & flags) const {
    flags[7] = endianness;
    flags[6] = multicast_flag;
  }
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

  static IPAddress get_array_from_address(uint32_t address) {
    IPAddress array_address;
    // TODO!
    for (size_t i = 0; i < 4; ++i) {
      array_address[i] = (address & (UINT64_MAX >> (8*i))) >> (32 - 8 * i);
    }
    return array_address;
  }
};

// Networking context.
// TODO formalize traits of "NetworkContext"
// Should this be a singleton? Yarr
class Context {
public:
  // TODO Nope, this can't be constexpr, needs to be determined by the Domain
  static constexpr LocatorUDPv4_t default_multicast_locator = {
    default_spdp_multicast_port(cmbml_test_domain_id),
    LocatorUDPv4_t::address_from_dot_notation({239, 255, 0, 1})
  };
  static constexpr LocatorUDPv4_t invalid_locator = {0, 0};
  static const int32_t kind = LOCATOR_KIND_UDPv4;

  Context();

  // Settings
  // This is pretty big...
  // using default_resend_data_period = Duration_t<30, 0>;

  void add_unicast_receiver(const Locator_t & locator);
  void add_multicast_receiver(const Locator_t & locator);

  void unicast_send(const Locator_t & locator, const uint32_t * packet, size_t size);

  void multicast_send(const Locator_t & locator, const uint32_t * packet, size_t size);

  template<typename CallbackT>
  void receive_packet(CallbackT && callback, size_t packet_size = CMBML__MAX_FRAGMENT_SIZE)
  {
    fd_set socket_set;
    FD_ZERO(&socket_set);
    int max_socket = 0;
    for (const auto & port_socket_pair : port_socket_map) {
      FD_SET(port_socket_pair.second, &socket_set);
      if (port_socket_pair.second > max_socket) {
        max_socket = port_socket_pair.second;
      }
    }

    int num_fds = select(max_socket + 1, &socket_set, NULL, NULL, NULL);
    for (const auto & port_socket_pair : port_socket_map) {
      Packet<> packet(packet_size);
      int recv_socket = port_socket_pair.second;
      if (!FD_ISSET(recv_socket, &socket_set)) {
        continue;
      }

      // TODO checking src is important error checking/security
      // struct sockaddr_in src_address;
      // 
      ssize_t bytes_received = recvfrom(
        recv_socket, packet.data(), packet.size() * sizeof(Packet<>::value_type), 0, NULL, NULL);
      callback(packet);
    }
  }

  const IPAddress & address_as_array() const;

private:

  static void socket_send(
    int socket, const Locator_t & locator, const uint32_t * packet, size_t size);

  // Blocks until a packet is received.
  // Intention is to wrap this in a future or async task in the executor.

  uint32_t local_address;  // what's the best type?
  IPAddress address_array;

  int unicast_send_socket = -1;
  int multicast_send_socket = -1;

  std::map<uint16_t, int> port_socket_map;

  // TODO: store and use allocator for Packets
};

}
}


#endif  // CMBML__PSM__UDP__CONTEXT_HPP_
