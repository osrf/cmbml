#ifndef CMBML__SPDP___HPP_
#define CMBML__SPDP___HPP_

#include <cmbml/dds/writer.hpp>
#include <cmbml/dds/reader.hpp>
#include <cmbml/discovery/participant/spdp_disco_data.hpp>
#include <cmbml/utility/option_map.hpp>

namespace cmbml {
  constexpr auto spdp_writer_options_map = hana::make_map(
    hana::make_pair(EndpointOptions::stateful, false),
    hana::make_pair(EndpointOptions::push_mode, true),
    hana::make_pair(EndpointOptions::reliability, ReliabilityKind_t::best_effort),
    hana::make_pair(EndpointOptions::topic_kind, TopicKind_t::with_key)
  );

  CMBML__MAKE_WRITER_OPTIONS(SPDPWriterOptions, spdp_writer_options_map);

  // The builtin endpoints have an empty topic name.
  class SpdpParticipantDataWriter :
    public dds::DataWriter<SpdpDiscoData, SPDPWriterOptions>
  {
  public:
    using ParentType = dds::DataWriter<SpdpDiscoData, SPDPWriterOptions>;

    SpdpParticipantDataWriter(Participant & p) : ParentType("", p) {
      message = p.create_discovery_data();
      rtps_writer.guid.entity_id = spdp_writer_id;
    };

    template<typename TransportT>
    StatusCode send_discovery_data(TransportT & transport) {
      // Set a few things in the message based on the transport?
      // Make a new message every time to keep it fresh
      SpdpDiscoData message = rtps_writer.participant.create_discovery_data();
      // TODO Do I need to set metatraffic_*_locator_list ?
      return write(std::move(message), transport);
    }

  private:
    SpdpDiscoData message;
  };

  constexpr auto spdp_reader_options = hana::make_map(
    hana::make_pair(EndpointOptions::stateful, false),
    hana::make_pair(EndpointOptions::expects_inline_qos, true),
    hana::make_pair(EndpointOptions::reliability, ReliabilityKind_t::best_effort),
    hana::make_pair(EndpointOptions::topic_kind, TopicKind_t::with_key),
    hana::make_pair(EndpointOptions::transport, hana::type_c<udp::Transport>)
  );

  CMBML__MAKE_READER_OPTIONS(SpdpReaderOptions, spdp_reader_options);

  class SpdpParticipantDataReader :
    public dds::DataReader<SpdpDiscoData, SpdpReaderOptions>
  {
  public:
    using ParentType = dds::DataReader<SpdpDiscoData, SpdpReaderOptions>;

    SpdpParticipantDataReader(Participant & p) : ParentType("", p) {
      rtps_reader.guid.entity_id = spdp_reader_id;
    }

  private:

  };

  // Should this be configurable?
  static const std::chrono::nanoseconds participant_data_resend_period(30*1000*1000);
}

#endif  // CMBML__SPDP___HPP_
