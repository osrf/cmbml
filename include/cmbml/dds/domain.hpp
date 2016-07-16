#ifndef CMBML__DDS__DOMAIN__HPP_
#define CMBML__DDS__DOMAIN__HPP_

#include <cmbml/types.hpp>

#include <cmbml/discovery/participant/spdp.hpp>
#include <cmbml/dds/waitset.hpp>

#include <cmbml/utility/console_print.hpp>

namespace cmbml {

struct DiscoveredParticipant {
  Participant p;
  // TODO rethink
  // representations of Local participants, the serializable Participant POD,
  // and Discovered participants
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

  // Maybe these functions should be in spdp.hpp
  template<typename Executor, typename Context>
  SpdpParticipantDataWriter &
  create_spdp_writer(Participant & p, Context & context) {
    known_spdp_writers.emplace_back(p);
    SpdpParticipantDataWriter & spdp_builtin_writer = known_spdp_writers.back();
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
  }

  template<typename Executor>
  SpdpParticipantDataReader &
  create_spdp_reader(Participant & p) {
    known_spdp_readers.emplace_back(p);
    SpdpParticipantDataReader & spdp_builtin_reader = known_spdp_readers.back();
    Executor & executor = Executor::get_instance();
    // Time for guard conditions and read conditions
    executor.add_task(
      [this, &spdp_builtin_reader](const auto & timeout) {
        // TODO Block until take is ready
        SpdpDiscoData disco_data;
        auto & read_condition = spdp_builtin_reader.create_read_condition();
        read_condition.wait_until_trigger(timeout);
        auto status = spdp_builtin_reader.take(disco_data);
        if (status != StatusCode::ok) {
          return status;
        }
        on_new_participant(std::move(disco_data));
        // TODO Do some stuff with this data
        // when does the read trigger value reset??
        return StatusCode::ok;
      }
    );

    // Callback configuration?

    return spdp_builtin_reader;
  }

  // Storage
  /*
  SedpPubWriter &
  create_sedp_pub_
  */

  // in the future, we could probably add some short-circuiting logic for endpoints
  // in the same participant!
  template<typename Executor, typename Context>
  Participant & create_new_participant(Context & transport_context) {
    List<Locator_t> multicast_locator_list = {transport_context.get_default_multicast_locator()};
    local_participants.emplace_back(
      get_next_guid_prefix(transport_context), std::move(multicast_locator_list));
    // TODO Assert that the guid we produced is not currently in local participants OR
    // discovered participants
    // Add builtin spdp endpoints to container
    create_spdp_writer<Executor>(local_participants.back(), transport_context);
    create_spdp_reader<Executor>(local_participants.back());

    // TODO add builtin sedp endpoints if they are enabled.

    /*
    create_sedp_pub_writer();
    create_sedp_pub_reader();
    create_sedp_sub_writer();
    create_sedp_sub_reader();
    create_sedp_topic_writer();
    create_sedp_topic_reader();
    */

    return local_participants.back();
  }

  // When a remote participant is discovered
  void on_new_participant(SpdpDiscoData && data) {
    for (const auto & p : discovered_participants) {
      if (p.guid.prefix == data.guid_prefix) {
        return;
      }
    }
    CMBML__DEBUG("We discovered a new participant!! Woohoo.\n");

    // for each local participant, add some stuff
    // TODO
    for (auto & local_participant : local_participants) {
      // Discover the new participant's builtin sedp publication reader
      if (data.available_builtin_endpoints[publications_reader]) {
        // Match the local participant's builtin sedp publication writer.
        // 
      }
      // How interesting, this pattern repeats
      if (data.available_builtin_endpoints[subscriptions_reader]) {
        // Match the local participant's builtin sedp subscription writer.
        // 
      }
    }

    discovered_participants.emplace_back(data);
  }

  // These would be helpful functions for combining constructing the object,
  // adding tasks to an Executor, and announcing it on the discovery protocol.
  /*
  template<typename TopicT, typename ReaderOptions>
  auto create_data_reader() {
  }

  template<typename TopicT, typename ReaderOptions>
  auto create_data_writer() {
  }
  */

  // TODO Disable if the BuiltinEndpointSet doesn't have a subscriptions_writer
  template<typename ReaderT>
  void announce_reader(Participant & participant, ReaderT & reader) {
    // Convert the Reader to DiscoReaderData
    // Publish on the local sedp_sub_writer associated with this participant
  }

  template<typename WriterT>
  void announce_writer(Participant & participant, WriterT & reader) {
    // Convert the Writer to DiscoWriterData
    // Publish on the local sedp_pub_writer
  }

  // TODO How/when to announce Topics?

  // When a new reader is discovered over SEDP, match it to the corresponding writer
  void on_new_reader(DiscoReaderData && reader) {
  }

  void on_new_writer(DiscoWriterData && writer) {
  }

  void on_topic_data(TopicBuiltinTopicData && writer) {
  }

private:
  Domain() {};
  Domain(const Domain &) = delete;
  Domain(Domain &&) = delete;

  // Process-local participants.
  List<Participant> local_participants;
  List<Participant> discovered_participants;

  // These represent local builtin endpoints.
  List<SpdpParticipantDataWriter> spdp_writers;
  List<SpdpParticipantDataReader> spdp_readers;

  // TODO fast lookup keyed on participant?
  List<SedpPubWriter> sedp_pub_writers;
  List<SedpPubReader> sedp_pub_readers;

  List<SedpSubWriter> sedp_sub_writers;
  List<SedpSubReader> sedp_sub_readers;

  List<SedpTopicWriter> sedp_topic_writers;
  List<SedpTopicReader> sedp_topic_readers;

  // Endpoints whose types are unknown to us.
  // I believe the domain needs a handle for even the locally created endpoints
  // so that it can signal the endpoints on discovery.
  List<dds::EndpointBase *> known_endpoints;

  // Come up with casting scheme based on the EntityKind
  // template parameters though...
  // Can't do this with the DDS types because they compose instead of inheriting
  // I guess DDS Reader and Writer should inherit from a common base
  // List<Endpoint> known_endpoints;

  // TODO Use this somewhere
  // uint32_t domain_id = cmbml_test_domain_id;
};

}

#endif  // CMBML__DDS__DOMAIN__HPP_
