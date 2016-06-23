#ifndef CMBML__PSM__UDP__LOCATORS_HPP_
#define CMBML__PSM__UDP__LOCATORS_HPP_

#include <cmbml/psm/udp/udp.hpp>
#include <cmbml/psm/udp/ports.hpp>

namespace cmbml {
namespace udp {
  static const LocatorUDPv4_t default_multicast_locator = {
    LOCATOR_KIND_UDPv4,
    default_spdp_multicast_port(cmbml_test_domain_id),
    address_from_dot_notation(239, 255, 0, 1)
  };

}
}

#endif  // CMBML__PSM__UDP__LOCATORS_HPP_
