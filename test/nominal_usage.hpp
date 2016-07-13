#ifndef CMBML__NOMINAL__USAGE_HPP_
#define CMBML__NOMINAL__USAGE_HPP_

#include <cmbml/utility/option_map.hpp>
#include <cmbml/structure/endpoint.hpp>

namespace cmbml {
namespace nominal_usage {
  namespace hana = boost::hana;

  constexpr auto writer_options = make_option_map(
    hana::make_pair(hana::type_c<EndpointOptions::stateful>, false),
    hana::make_pair(hana::type_c<EndpointOptions::reliability>, ReliabilityKind_t::best_effort),
    hana::make_pair(hana::type_c<EndpointOptions::topic_kind>, TopicKind_t::with_key),
    hana::make_pair(hana::type_c<EndpointOptions::push_mode>, true)
  );

}
}

#endif // CMBML__NOMINAL__USAGE_HPP_
