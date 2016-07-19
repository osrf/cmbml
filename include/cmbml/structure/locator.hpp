#ifndef CMBML__STRUCTURE__LOCATOR_HPP_
#define CMBML__STRUCTURE__LOCATOR_HPP_

#include <boost/hana/define_struct.hpp>

namespace cmbml {

  using IPAddress = std::array<Octet, 16>;

  struct Locator_t {
    BOOST_HANA_DEFINE_STRUCT(Locator_t,
    (int32_t, kind),
    (uint32_t, port),
    (IPAddress, address));
  };

}  // namespace cmbml

#endif  // CMBML__STRUCTURE__LOCATOR_HPP_
