#ifndef CMBML__DDS__DOMAIN__HPP_
#define CMBML__DDS__DOMAIN__HPP_

#include <cmbml/types.hpp>

#include <cmbml/discovery/participant/spdp.hpp>

namespace cmbml {

struct DiscoveredParticipant {
  Participant p;

};

class Domain {
public:
  static Domain & get_instance() {
    static Domain instance;
    return instance;
  }

  template<typename Context>
  GuidPrefix_t get_next_guid_prefix(Context & context) {
    GuidPrefix_t guid_prefix;
    guid_prefix[0] = cmbml_vendor_id[0];
    guid_prefix[1] = cmbml_vendor_id[1];
    // Choose the GUID prefix values based on the current time, our local IP,
    // and a random hash
    // TODO This is IP specific. Is Context required to provide an IP address?
    // we have 10 bytes of info
    // IP is either 6 or 4 octets
    // prefix 2-8: Take the most significant 6 bytes of time
    uint64_t time = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();

    // Danger zone: hardcoded values
    for (size_t i = 0; i < 6; ++i) {
      guid_prefix[i + 2] = (time & (UINT64_MAX >> (8 * i))) >> (48 - 8 * i);
    }

    auto ip_address = context.address_as_array();
    for (size_t i = 0; i < 4; ++i) {
      guid_prefix[i + 8] = ip_address[i];
    }

    return guid_prefix;
  }

  // Maybe these functions should bein spdp.hpp
  template<typename Executor, typename Context>
  static auto create_spdp_writer(Participant & p, Context & context) {
    SpdpParticipantDataWriter spdp_builtin_writer(p);
    Executor & executor = Executor::get_instance();
    // Callback configuration:
    // The SpdpParticipantDataWriter needs to periodically send the
    // SpdpDiscoData representing this datawriter
    executor.add_timed_task(participant_data_resend_period, false,
      [&spdp_builtin_writer, &context]() {
        spdp_builtin_writer.send_discovery_data(context);
      }
    );

    return spdp_builtin_writer;
    // Push onto endpoints list
  }

  template<typename Executor>
  static auto create_spdp_reader(Participant & p) {
    SpdpParticipantDataReader spdp_builtin_reader(p);
    Executor & executor = Executor::get_instance();
    // Time for guard conditions and read conditions
    /*
    executor.add_task(
      [](&spdp_builtin_reader) {
        // TODO Block until take is ready
        List<SpdpDiscoData> disco_data;
        spdp_builtin_reader.take(disco_data);
      }
    );
    */

    // Callback configuration?

    return spdp_builtin_reader;
  }

  template<typename Context, typename Executor>
  Participant & create_new_participant(Context & transport_context) {
    known_participants.emplace_back(
      get_next_guid_prefix(transport_context), {transport_context.default_multicast_locator});
    // Add builtin endpoints to container
    auto spdp_writer = Domain::create_spdp_writer<Executor>(
      known_participants.back(), transport_context);
    auto spdp_reader = Domain::create_spdp_reader<Executor>(
      known_participants.back(), transport_context);

    // add builtin sedp endpoints

  }

  // When a remote participant is discovered
  void on_new_participant(SpdpDiscoData && data) {
    for (const auto & p : known_participants) {
      if (p.guid.prefix == data.guid_prefix) {
        return;
      }
    }
    known_participants.emplace_back(data);

  }

private:
  Domain() {};
  Domain(const Domain &) = delete;
  Domain(Domain &&) = delete;

  List<Participant> known_participants;

  // Come up with casting scheme based on the EntityKind
  // template parameters though...
  // Can't do this with the DDS types because they compose instead of inheriting
  // I guess DDS Reader and Writer should inherit from a common base
  // List<Endpoint> known_endpoints;

  uint32_t domain_id = cmbml_test_domain_id;
};

}

#endif  // CMBML__DDS__DOMAIN__HPP_
