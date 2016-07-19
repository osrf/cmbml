#ifndef CMBML__STRUCTURE__LOCATOR_HPP_
#define CMBML__STRUCTURE__LOCATOR_HPP_

#include <boost/hana/define_struct.hpp>

namespace cmbml {

  using IPAddress = std::array<Octet, 16>;

  // does this need to indicate multicast or unicast?
  // otherwise how to know what to do on send?
  enum struct LocatorKind : int32_t {
    invalid = -1,
    reserved = 0,
    udpv4 = 1,
    udpv6 = 2
  };

  struct Locator_t {
    BOOST_HANA_DEFINE_STRUCT(Locator_t,
    (LocatorKind, kind),
    (uint32_t, port),
    (IPAddress, address));
  };

}  // namespace cmbml

#endif  // CMBML__STRUCTURE__LOCATOR_HPP_
