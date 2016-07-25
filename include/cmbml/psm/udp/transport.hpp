#ifndef CMBML__PSM__UDP__TRANSPORT_HPP_
#define CMBML__PSM__UDP__TRANSPORT_HPP_

#include <cmbml/common.hpp>
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
    const std::array<Octet, 4> addr = {{locator.address[0], locator.address[1],
                                       locator.address[2], locator.address[3]}};
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
  constexpr static IPAddress get_array_from_address(const std::array<Octet, 4> & adr) {
    return {{0, 0, 0, 0, 0, 0, 0, 0, adr[0], adr[1], adr[2], adr[3]}};
  }
};

// TODO formalize traits of "NetworkTransport"
class Transport {
public:
  static const LocatorKind kind = LocatorKind::udpv4;

  constexpr static Locator_t get_default_unicast_locator(uint32_t p_id) {
    return {
      LocatorKind::udpv4,
      static_cast<uint32_t>(udp::default_user_unicast_port(cmbml_default_domain_id, p_id)),
      udp::LocatorUDPv4_t::get_array_from_address({{239, 255, 0, 1}})
    };
  }

  constexpr static Locator_t get_default_multicast_locator() {
    return {
      LocatorKind::udpv4,
      static_cast<uint32_t>(udp::default_spdp_multicast_port(cmbml_default_domain_id)),
      udp::LocatorUDPv4_t::get_array_from_address({{239, 255, 0, 1}})
    };
  }
  constexpr static LocatorUDPv4_t invalid_locator = {0, 0};

  Transport();

  // Settings
  // This is pretty big...
  // using default_resend_data_period = Duration_t<30, 0>;

  void add_unicast_receiver(const Locator_t & locator);
  void add_multicast_receiver(const Locator_t & locator);

  void unicast_send(const Locator_t & locator, const uint32_t * packet, size_t size);

  void multicast_send(const Locator_t & locator, const uint32_t * packet, size_t size);

  template<typename CallbackT>
  StatusCode receive_packet(CallbackT && callback, const std::chrono::nanoseconds & timeout,
      size_t packet_size = CMBML__MAX_FRAGMENT_SIZE)
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

    struct timespec * timeout_struct = NULL;
    struct timespec timeout_value;
    if (timeout.count() > 0) {
      timeout_value.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
      timeout_value.tv_nsec = (timeout -
          std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::seconds(timeout_value.tv_sec))).count();
      timeout_struct = &timeout_value;
    }
    int num_fds = pselect(max_socket + 1, &socket_set, NULL, NULL, timeout_struct, NULL);
    (void) num_fds;
    for (const auto & port_socket_pair : port_socket_map) {
      Packet<> packet(packet_size);
      int recv_socket = port_socket_pair.second;
      if (!FD_ISSET(recv_socket, &socket_set)) {
        continue;
      }

      // TODO checking src is important error checking/security
      // struct sockaddr_in src_address;
      ssize_t bytes_received = recvfrom(
        recv_socket, packet.data(), packet.size() * sizeof(Packet<>::value_type), 0, NULL, NULL);
      if (bytes_received != 0) {
        callback(packet);
      }
    }
    return StatusCode::ok;
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


#endif  // CMBML__PSM__UDP__TRANSPORT_HPP_
