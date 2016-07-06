#ifndef CMBML__DDS__DOMAIN__HPP_
#define CMBML__DDS__DOMAIN__HPP_

#include <cmbml/types.hpp>

#include <cmbml/discovery/participant/spdp.hpp>

namespace cmbml {

class Domain {
public:
  static Domain & get_instance() {
    static Domain instance;
    return instance;
  }

  GuidPrefix_t get_next_guid_prefix() {
    GuidPrefix_t guid_prefix;
    guid_prefix[0] = cmbml_vendor_id[0];
    guid_prefix[1] = cmbml_vendor_id[1];
    // Choose the 
    return guid_prefix;
  }

  template<typename Context>
  Participant & create_new_participant(Context & transport_context) {
    known_participants.emplace_back(
      get_next_guid_prefix(), {transport_context.default_multicast_locator});
    // Add builtin endpoints to container
    SpdpParticipantDataWriter<Context> spdp_builtin_writer(known_participants.back());
    SpdpParticipantDataReader<Context> spdp_builtin_reader(known_participants.back());
    // add builtin sedp endpoints.

  }

  // When a remote participant is discovered
  void on_new_participant(SpdpDiscoData && data) {
    known_participants.emplace_back(data);
    // May need to
  }

private:
  Domain() {};
  Domain(const Domain &) = delete;
  Domain(Domain &&) = delete;

  List<Participant> known_participants;
  // Come up with casting scheme based on the EntityKind
  // template parameters though...
  // Can't do this with the DDS types
  // Perhaps Reader and Writer should inherit from a common base
  // List<Endpoint> known_endpoints;
  uint32_t domain_id = cmbml_test_domain_id;
};


}

#endif  // CMBML__DDS__DOMAIN__HPP_
