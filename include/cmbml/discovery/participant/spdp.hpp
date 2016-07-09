#ifndef CMBML__SPDP___HPP_
#define CMBML__SPDP___HPP_

#include <cmbml/dds/writer.hpp>
#include <cmbml/dds/reader.hpp>

#include <cmbml/discovery/participant/spdp_disco_data.hpp>

namespace cmbml {
  class SpdpParticipantDataWriter :
    public dds::DataWriter<SpdpDiscoData, StatelessWriter<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>>
  {
  public:
    using ParentType = dds::DataWriter<SpdpDiscoData, StatelessWriter<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>>;

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
      write(message, this->instance_handle, context);
    }

  private:
    SpdpDiscoData message;
  };


  class SpdpParticipantDataReader :
    public dds::DataReader<SpdpDiscoData, StatelessReader<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>>
  {
  public:
    using ParentType = dds::DataReader<SpdpDiscoData, StatelessReader<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>>;

    SpdpParticipantDataReader(Participant & p) : ParentType(p) {
      rtps_reader.guid.entity_id = spdp_reader_id;
    }

  private:

  };

  // Should this be configurable?
  static const std::chrono::nanoseconds participant_data_resend_period(30*1000*1000);
}

#endif  // CMBML__SPDP___HPP_
