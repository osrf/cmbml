#ifndef CMBML__SPDP___HPP_
#define CMBML__SPDP___HPP_

#include <cmbml/dds/writer.hpp>
#include <cmbml/dds/reader.hpp>

#include <cmbml/discovery/participant/spdp_disco_data.hpp>

namespace cmbml {
  // TODO need to set their EntityKinds accordingly on initialization.
  // We may need to end up inheriting instead because of custom behavior.
  // Spec needs
  using SpdpParticipantDataWriter = dds::DataWriter<SpdpDiscoData,
        StatelessWriter<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>>;

  using SpdpParticipantDataReader = dds::DataReader<SpdpDiscoData,
        StatelessReader<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>>;

  // Should this be configurable?
  static const std::chrono::nanoseconds participant_data_resend_period(30*1000*1000);
}

#endif  // CMBML__SPDP___HPP_
