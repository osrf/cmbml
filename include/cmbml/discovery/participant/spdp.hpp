#ifndef CMBML__SPDP___HPP_
#define CMBML__SPDP___HPP_

#include <cmbml/dds/writer.hpp>
#include <cmbml/dds/reader.hpp>
#include <cmbml/discovery/participant/spdp_disco_data.hpp>
#include <cmbml/utility/option_map.hpp>

namespace cmbml {
  constexpr auto spdp_writer_options = make_option_map(
    hana::make_pair(hana::type_c<EndpointOptions::stateful>, false),
    hana::make_pair(hana::type_c<EndpointOptions::push_mode>, true),
    hana::make_pair(hana::type_c<EndpointOptions::reliability>, ReliabilityKind_t::best_effort),
    hana::make_pair(hana::type_c<EndpointOptions::topic_kind>, TopicKind_t::with_key)
  );

  class SpdpParticipantDataWriter :
    public dds::DataWriter<SpdpDiscoData, decltype(spdp_writer_options), spdp_writer_options>
  {
  public:
    using ParentType = dds::DataWriter<
      SpdpDiscoData, decltype(spdp_writer_options), spdp_writer_options>;

    SpdpParticipantDataWriter(Participant & p) : ParentType(p) {
      message = p.create_discovery_data();
      rtps_writer.guid.entity_id = spdp_writer_id;
    };

    template<typename Context>
    StatusCode send_discovery_data(Context & context) {
      // Set a few things in the message based on the context?
      // Make a new message every time to keep it fresh
      SpdpDiscoData message = rtps_writer.participant.create_discovery_data();
      // TODO Do I need to set metatraffic_*_locator_list ?
      return write(std::move(message), this->instance_handle, context);
    }

  private:
    SpdpDiscoData message;
  };

  constexpr auto spdp_reader_options = make_option_map(
    hana::make_pair(hana::type_c<EndpointOptions::stateful>, false),
    hana::make_pair(hana::type_c<EndpointOptions::expects_inline_qos>, true),
    hana::make_pair(hana::type_c<EndpointOptions::reliability>, ReliabilityKind_t::best_effort),
    hana::make_pair(hana::type_c<EndpointOptions::topic_kind>, TopicKind_t::with_key),
    hana::make_pair(hana::type_c<EndpointOptions::transport>, hana::type_c<udp::Context>)
  );

  class SpdpParticipantDataReader :
    public dds::DataReader<SpdpDiscoData, decltype(spdp_reader_options), spdp_reader_options>
  {
  public:
    using ParentType = dds::DataReader<SpdpDiscoData,
          decltype(spdp_reader_options), spdp_reader_options>;

    SpdpParticipantDataReader(Participant & p) : ParentType(p) {
      rtps_reader.guid.entity_id = spdp_reader_id;
    }

  private:

  };

  // Should this be configurable?
  static const std::chrono::nanoseconds participant_data_resend_period(30*1000*1000);
}

#endif  // CMBML__SPDP___HPP_
