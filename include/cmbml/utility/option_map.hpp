#ifndef CMBML__DDS__OPTION_MAP_HPP_
#define CMBML__DDS__OPTION_MAP_HPP_

#include <boost/hana/map.hpp>

namespace cmbml {

  // TODO internal linkage problem
  // This should give utilities for converting option keys into the map
  template<typename ...Options>
  constexpr auto make_option_map(Options && ...options) {
    namespace hana = boost::hana;
    return hana::make_map(
        options...
    );
  }

}  // namespace cmbml

#endif  // CMBML__DDS__OPTION_MAP_HPP_
