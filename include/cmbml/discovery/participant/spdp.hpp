#ifndef CMBML__SPDP___HPP_
#define CMBML__SPDP___HPP_

#include <cmbml/dds/writer.hpp>
#include <cmbml/dds/reader.hpp>

#include <cmbml/discovery/participant/spdp_disco_data.hpp>

namespace cmbml {
  // TODO these need to set their EntityKinds accordingly on initialization.
  // We may need to end up inheriting instead.
  template<typename Context, typename Executor = SyncExecutor>
  using SpdpParticipantDataWriter = dds::DataWriter<SpdpDiscoData,
        StatelessWriter<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>,
      Context>;

  template<typename Context, typename Executor = SyncExecutor>
  using SpdpParticipantDataReader = dds::DataReader<SpdpDiscoData,
        StatelessReader<true,
          EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>,
      Context>;
}

#endif  // CMBML__SPDP___HPP_
