#include <cmbml/psm/udp/context.hpp>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>

#include <sys/select.h>
#include <sys/socket.h>

using namespace cmbml;

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025  // ???
#endif

udp::Context::Context() {
  // alternatively we could do delayed initialization
  // might prefer this since syscalls could fail
  // Unicast socket
  unicast_send_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (unicast_send_socket < 0) {
    // Fatal error
    return;
  }

  // Initialize a multicast socket
  multicast_send_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (multicast_send_socket < 0) {
    // Fatal error
    return;
  }

  // Set to multicast socket options
  // need to get our address
  struct ifaddrs * addresses = nullptr;
  if (getifaddrs(&addresses) == -1) {
    // TODO Error handling
    return;
  }
  std::string address_str = "127.0.0.1";
  // TODO Check # of interfaces and warn user which one we're picking if multiple
  for (struct ifaddrs * address = addresses; address; address = address->ifa_next) {
    if (!address->ifa_addr) {
      continue;
    }
    int family = address->ifa_addr->sa_family;
    if (family != AF_INET) {
      continue;
    }
    char host[NI_MAXHOST];
    memset(host, 0, NI_MAXHOST);
    if (getnameinfo(
          address->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST))
    {
      // getnameinfo returns 0 on success, so non-zero value is an error
      continue;
    }
    if (strcmp(host, "127.0.0.1") == 0) {
        continue;
    }
    address_str = host;
    break;
  }
  struct sockaddr_in local_sockaddr;
  memset(&local_sockaddr, 0, sizeof(sockaddr_in));
  local_sockaddr.sin_addr.s_addr = inet_addr(address_str.c_str());
  local_address = local_sockaddr.sin_addr.s_addr;
  freeifaddrs(addresses);

  int result = setsockopt(
    multicast_send_socket, IPPROTO_IP, IP_MULTICAST_IF,
    reinterpret_cast<char *>(&local_sockaddr), sizeof(struct sockaddr_in));
  if (result < 0) {
    // fatal error condition
    return;
  }

  uint8_t loop = 1;
  result = setsockopt(multicast_send_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
  if (result < 0) {
    // fatal error
    return;
  }

}


void udp::Context::add_unicast_receiver(const Locator_t & locator) {
  // TODO Convert locator_t to LocatorUDPv4_t here and below?
  // Create and bind the receive sockets
  int recv_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (recv_socket < 0) {
    return;
  }
  struct sockaddr_in receive_bind_address;
  memset(&receive_bind_address, 0, sizeof(sockaddr_in));
  receive_bind_address.sin_family = AF_INET;
  receive_bind_address.sin_addr.s_addr = local_address;
  receive_bind_address.sin_port = htons(locator.port);
  int result = bind(recv_socket, reinterpret_cast<struct sockaddr *>(&receive_bind_address),
      sizeof(receive_bind_address));
  if (result < 0) {
    shutdown(recv_socket, 0);
    return;
  }
  port_socket_map[locator.port] = recv_socket;
}

// Expect locator to contain the port to listen on and the address representing the 
// multicast group
void udp::Context::add_multicast_receiver(const Locator_t & locator) {
  const udp::LocatorUDPv4_t locator_v4(locator);
  // Create and bind the receive sockets
  int recv_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (recv_socket < 0) {
    return;
  }
  struct sockaddr_in receive_bind_address;
  memset(&receive_bind_address, 0, sizeof(sockaddr_in));
  receive_bind_address.sin_family = AF_INET;
  receive_bind_address.sin_addr.s_addr = htonl(INADDR_ANY);
  receive_bind_address.sin_port = htons(locator_v4.port);
  int result = bind(recv_socket, reinterpret_cast<struct sockaddr *>(&receive_bind_address),
      sizeof(receive_bind_address));
  if (result < 0) {
    shutdown(recv_socket, 0);
    return;
  }
  // Set socket options to multicast
  struct ip_mreq membership_requirements;
  membership_requirements.imr_multiaddr.s_addr = locator_v4.address;
  membership_requirements.imr_interface.s_addr = local_address;
  result = setsockopt(
    recv_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &membership_requirements, sizeof(ip_mreq));
  if (result < 0) {
    return;
  }

  port_socket_map[locator_v4.port] = recv_socket;
}

void udp::Context::unicast_send(const Locator_t & locator, const uint32_t * packet, size_t size) {
  socket_send(unicast_send_socket, locator, packet, size);
}

void udp::Context::multicast_send(const Locator_t & locator, const uint32_t * packet, size_t size) {
  socket_send(multicast_send_socket, locator, packet, size);
}

void udp::Context::socket_send(
  int sender_socket, const Locator_t & locator, const uint32_t * packet, size_t size)
{
  if (sender_socket == -1) {
    // Socket isn't open yet, so we can't send.
    return;
  }
  const udp::LocatorUDPv4_t locator_v4(locator);

  struct sockaddr_in dest_address;
  dest_address.sin_port = htons(static_cast<uint32_t>(locator_v4.port));
  dest_address.sin_addr.s_addr = static_cast<uint32_t>(locator_v4.address);

  sendto(sender_socket, packet, size, 0,
      reinterpret_cast<struct sockaddr *>(&dest_address), sizeof(dest_address));
}


// Maybe have a timeout option for select?
// packet size has to be smaller for e.g. STM32F0
// so maybe for that Context we will need to make max packet size a type trait
template<typename CallbackT>
void udp::Context::receive_packet(CallbackT & callback, size_t packet_size) {
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
