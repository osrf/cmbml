#ifndef CMBML__STRUCTURE__ENDPOINT_HPP_
#define CMBML__STRUCTURE__ENDPOINT_HPP_

#include <cmbml/types.hpp>
#include <cmbml/structure/participant.hpp>

namespace cmbml {

  template<ReliabilityKind_t reliabilityLevel, TopicKind_t topicKind>
  struct EndpointParams {
    static const ReliabilityKind_t reliability_level = reliabilityLevel;
    static const TopicKind_t topic_kind = topicKind;
  };

  // TODO Template error checking
  // On initialization, initialize Transport-specific data with this Endpoint.
  // An endpoint needs a participant to be instantiated.
  template<typename EndpointParams>
  struct Endpoint : Entity {
    // List of locators that can receive messages for this endpoint.
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
    static const ReliabilityKind_t reliability_level = EndpointParams::reliability_level;
    static const TopicKind_t topic_kind = EndpointParams::topic_kind;

    // Not in the spec: Cache a reference to our participant
    // TODO ensure that Participant can never go out of scope while Endpoint is alive
    const Participant & participant;

    explicit Endpoint(Participant & p) : participant(p) {
      unicast_locator_list = p.default_unicast_locator_list;
      multicast_locator_list = p.default_multicast_locator_list;
      guid.prefix = p.guid.prefix;
    }
  };


}  // namespace cmbml

#endif  // CMBML__STRUCTURE__ENDPOINT_HPP_
