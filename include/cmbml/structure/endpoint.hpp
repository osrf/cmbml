#ifndef CMBML__STRUCTURE__ENDPOINT_HPP_
#define CMBML__STRUCTURE__ENDPOINT_HPP_

#include <cmbml/types.hpp>
#include <cmbml/structure/participant.hpp>

#include <boost/hana/at_key.hpp>

namespace cmbml {

struct EndpointOptions {
  static constexpr auto stateful = hana::int_c<0>;
  static constexpr auto reliability = hana::int_c<1>;
  static constexpr auto topic_kind = hana::int_c<2>;
  static constexpr auto push_mode = hana::int_c<3>;
  static constexpr auto expects_inline_qos = hana::int_c<4>;
  static constexpr auto transport = hana::int_c<5>;
  static constexpr auto executor = hana::int_c<6>;
};

  // TODO Template error checking
  // On initialization, initialize Transport-specific data with this Endpoint.
  // An endpoint needs a participant to be instantiated.
  template<typename WriterOptions>
  struct Endpoint : Entity {
    // List of locators that can receive messages for this endpoint.
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
    static const ReliabilityKind_t reliability = WriterOptions::reliability;
    static const TopicKind_t topic_kind = WriterOptions::topic_kind;

    // Not in the spec: Cache a reference to our participant
    // TODO ensure that Participant can never go out of scope while Endpoint is alive
    const Participant & participant;

    Endpoint(const Participant & p) :
      participant(p)
    {
      // unicast_locator_list = p.default_unicast_locator_list;
      // multicast_locator_list = p.default_multicast_locator_list;
      guid.prefix = p.guid.prefix;
    }
  };


}  // namespace cmbml

#endif  // CMBML__STRUCTURE__ENDPOINT_HPP_
