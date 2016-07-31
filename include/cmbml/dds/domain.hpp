#ifndef CMBML__DDS__DOMAIN__HPP_
#define CMBML__DDS__DOMAIN__HPP_

#include <cmbml/types.hpp>

#include <cmbml/discovery/participant/spdp.hpp>
#include <cmbml/discovery/endpoint/sedp.hpp>
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

  template<typename TransportT>
  GuidPrefix_t get_next_guid_prefix(TransportT & transport) {
    GuidPrefix_t guid_prefix;
    guid_prefix[0] = cmbml_vendor_id[0];
    guid_prefix[1] = cmbml_vendor_id[1];
    // Choose the GUID prefix values based on the current time, our local IP,
    // and a random hash
    // TODO This is IP specific. Is TransportT required to provide an IP address?
    // we have 10 bytes of info
    // IP is either 6 or 4 octets
    // prefix 2-8: Take the most significant 6 bytes of time
    uint64_t time = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();

    // Danger zone: hardcoded values
    for (size_t i = 0; i < 6; ++i) {
      guid_prefix[i + 2] = (time & (UINT64_MAX >> (8 * i))) >> (48 - 8 * i);
    }

    auto ip_address = transport.address_as_array();
    for (size_t i = 0; i < 4; ++i) {
      guid_prefix[i + 8] = ip_address[i];
    }

    return guid_prefix;
  }

  // Maybe these functions should be in spdp.hpp
  template<typename Executor, typename TransportT>
  SpdpParticipantDataWriter &
  create_spdp_writer(Participant & p, TransportT & transport) {
    spdp_writers.emplace_back(p);
    SpdpParticipantDataWriter & spdp_builtin_writer = spdp_writers.back();
    Executor & executor = Executor::get_instance();
    spdp_builtin_writer.add_tasks(transport, executor);
    // Callback configuration:
    // The SpdpParticipantDataWriter needs to periodically send the
    // SpdpDiscoData representing this datawriter
    executor.add_timed_task(participant_data_resend_period, false,
      [&spdp_builtin_writer, &transport]() {
        CMBML__DEBUG("Resending discovery data.\n");
        spdp_builtin_writer.send_discovery_data(transport);
      },
      true  // trigger on creation
    );

    return spdp_builtin_writer;
  }

  template<typename Executor, typename BuiltinReader>
  void
  configure_builtin_reader(BuiltinReader & builtin_reader) {
    Executor & executor = Executor::get_instance();
    // Time for guard conditions and read conditions
    executor.add_task(
      [this, &builtin_reader](const auto & timeout) {
        typename BuiltinReader::Topic message;
        auto & read_condition = builtin_reader.create_read_condition();
        CMBML__DEBUG("Waiting on read condition for builtin reader\n");
        read_condition.wait_until_trigger(timeout);
        auto status = builtin_reader.take(message);
        if (status != StatusCode::ok) {
          return status;
        }
        CMBML__DEBUG("builtin reader message came through! Doing stuff.\n");
        on_builtin_data(std::move(message));
        // TODO Do some stuff with this data
        // when does the read trigger value reset??
        return StatusCode::ok;
      }
    );
  }

  template<typename Executor, typename TransportT>
  SpdpParticipantDataReader & create_spdp_reader(Participant & p, TransportT & transport) {
    spdp_readers.emplace_back(p);
    SpdpParticipantDataReader & spdp_reader = spdp_readers.back();
    Executor & executor = Executor::get_instance();
    spdp_reader.add_tasks(transport, executor);
    configure_builtin_reader<Executor>(spdp_reader);
    return spdp_reader;
  }

  template<typename Executor>
  SedpPubReader &
  create_sedp_pub_reader(Participant & p) {
    sedp_pub_readers.emplace_back(p);
    SedpPubReader & builtin_reader = sedp_pub_readers.back();
    configure_builtin_reader<Executor>(builtin_reader);
    return builtin_reader;
  }

  template<typename Executor>
  SedpSubReader &
  create_sedp_sub_reader(Participant & p) {
    sedp_sub_readers.emplace_back(p);
    SedpSubReader & builtin_reader = sedp_sub_readers.back();
    configure_builtin_reader<Executor>(builtin_reader);
    return builtin_reader;
  }

  template<typename Executor>
  SedpTopicsReader &
  create_sedp_topic_reader(Participant & p) {
    sedp_topic_readers.emplace_back(p);
    SedpTopicsReader & builtin_reader = sedp_topic_readers.back();
    configure_builtin_reader<Executor>(builtin_reader);
    return builtin_reader;
  }

  // in the future, we could probably add some short-circuiting logic for endpoints
  // in the same participant!
  template<typename Executor, typename TransportT>
  Participant & create_new_participant(TransportT & transport) {
    List<Locator_t> multicast_locator_list = {transport.get_default_multicast_locator()};
    local_participants.emplace_back(
      get_next_guid_prefix(transport),
      std::move(multicast_locator_list),
      participant_port_id++);
    // TODO Assert that the guid we produced is not currently in local participants OR
    // discovered participants
    // Add builtin spdp endpoints to container
    Participant & p = local_participants.back();
    create_spdp_writer<Executor>(p, transport);
    create_spdp_reader<Executor>(p, transport);

    // TODO Disable certain endpoints according to set options.
    sedp_pub_writers.emplace_back(p);
    sedp_sub_writers.emplace_back(p);
    sedp_topic_writers.emplace_back(p);

    // Readers
    create_sedp_pub_reader<Executor>(p);
    create_sedp_sub_reader<Executor>(p);
    create_sedp_topic_reader<Executor>(p);

    return p;
  }

  template<typename T>
  void on_builtin_data(T && data);

  template<typename EndpointT>
  void
  match_builtin_endpoint(
    GuidPrefix_t guid_prefix, SpdpDiscoData & data, List<EndpointT> & endpoints)
  {
    if (data.available_builtin_endpoints[static_cast<size_t>(EndpointT::kind)]) {
      // Find the endpoint that matches
      for (auto & endpoint : endpoints) {
        if (endpoint.get_guid().prefix == guid_prefix) {
          endpoint.match_proxy({data.guid_prefix, EndpointT::get_id()}, data);
          break;
        }
      }
    }
  }

  // When a remote participant is discovered
  void on_builtin_data(SpdpDiscoData && data) {
    for (const auto & p : discovered_participants) {
      if (p.guid.prefix == data.guid_prefix) {
        return;
      }
    }
    CMBML__DEBUG("We discovered a new participant!! Woohoo.\n");

    for (auto & local_participant : local_participants) {
      // Discover the new participant's builtin sedp publication reader
      // TODO metatraffic_*cast_locator_list!!!
      match_builtin_endpoint(local_participant.guid.prefix, data, sedp_pub_readers);
      match_builtin_endpoint(local_participant.guid.prefix, data, sedp_pub_writers);

      match_builtin_endpoint(local_participant.guid.prefix, data, sedp_sub_readers);
      match_builtin_endpoint(local_participant.guid.prefix, data, sedp_sub_writers);

      match_builtin_endpoint(local_participant.guid.prefix, data, sedp_topic_readers);
      match_builtin_endpoint(local_participant.guid.prefix, data, sedp_topic_writers);
    }

    discovered_participants.emplace_back(data);
  }
  // TODO How/when to announce Topics?

  // When a new reader is discovered over SEDP, match it to the corresponding writer
  // This is where we need ownership of the participant's known endpoints.
  void on_builtin_data(DiscoReaderData && reader) {
    CMBML__DEBUG("We discovered a new reader!! Woohoo.\n");
    // find the writer in the type-erased known endpoints and cast based on type info?
    // match a writer to this reader
    for (auto writer : writers) {
      assert(writer);
      // TODO be more responsible and check more metadata before matching
      if (writer->get_topic_name() == reader.subscription_data.topic_name) {
        writer->match_proxy(std::move(reader.reader_proxy));
        CMBML__DEBUG("We matched a new reader with a writer!! Woohoo.\n");
        // We could match multiple writers on the same topic, so don't break
      }
    }
  }

  void on_builtin_data(DiscoWriterData && writer) {
    CMBML__DEBUG("We discovered a new writer!! Woohoo.\n");
    for (auto reader : readers) {
      assert(reader);
      // TODO be more responsible and check more metadata before matching
      if (reader->get_topic_name() == writer.publication_data.topic_name) {
        reader->match_proxy(std::move(writer.writer_proxy));
        CMBML__DEBUG("We matched a new writer with a reader!! Woohoo.\n");
        // We could match multiple readers on the same topic, so don't break
      }
    }
  }

  void on_builtin_data(TopicBuiltinTopicData && writer) {
    // TODO
  }

  // These would be helpful functions for combining constructing the object,
  // adding tasks to an Executor, and announcing it on the discovery protocol.
  // hurrggh
  template<typename TopicT, typename ReaderOptions, typename Executor, typename TransportT>
  auto create_data_reader(const String & topic_name, Participant & p, TransportT & transport) {
    using ReaderT = dds::DataReader<TopicT, ReaderOptions>;
    ReaderT reader = ReaderT(topic_name, p);
    Executor & executor = Executor::get_instance();
    reader.add_tasks(transport, executor);
    // TODO clean up our entry in this vector in destructor
    readers.push_back(&reader);
    announce_reader(reader, p, transport);
    return reader;
  }

  template<typename TopicT, typename WriterOptions, typename Executor, typename TransportT>
  auto create_data_writer(const String & topic_name, Participant & p, TransportT & transport) {
    using WriterT = dds::DataWriter<TopicT, WriterOptions>;
    WriterT writer = WriterT(topic_name, p);
    Executor & executor = Executor::get_instance();
    writer.add_tasks(transport, executor);
    // TODO clean up our entry in this vector in destructor
    writers.push_back(&writer);
    announce_writer(writer, p, transport);
    return writer;
  }

  template<typename ReaderT, typename TransportT>
  void announce_reader(ReaderT & reader, Participant & participant, TransportT & transport) {
    // Convert the Reader to DiscoReaderData
    // Publish on the local sedp_sub_writer associated with this participant
    for (auto & builtin_writer : sedp_sub_writers) {
      if (builtin_writer.get_guid().prefix == participant.guid.prefix) {
        builtin_writer.write(reader.convert_to_reader_data(), transport);
        CMBML__DEBUG("Announcing created reader\n");
        break;
      }
    }
  }

  template<typename WriterT, typename TransportT>
  void announce_writer(WriterT & writer, Participant & participant, TransportT & transport) {
    // Convert the Writer to DiscoWriterData
    // Publish on the local sedp_pub_writer
    for (auto & builtin_writer : sedp_pub_writers) {
      if (builtin_writer.get_guid().prefix == participant.guid.prefix) {
        builtin_writer.write(writer.convert_to_writer_data(), transport);
        CMBML__DEBUG("Announcing created writer\n");
        break;
      }
    }
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

  List<SedpTopicsWriter> sedp_topic_writers;
  List<SedpTopicsReader> sedp_topic_readers;

  List<dds::ReaderBase *> readers;
  List<dds::WriterBase *> writers;

  // TODO Use this somewhere
  uint32_t domain_id = cmbml_default_domain_id;
  uint32_t participant_port_id = 0;
};

}

#endif  // CMBML__DDS__DOMAIN__HPP_
