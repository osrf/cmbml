#ifndef CMBML__SPDP___HPP_
#define CMBML__SPDP___HPP_

#include <cmbml/structure/writer.hpp>
#include <cmbml/structure/reader.hpp>

namespace cmbml {

  // not sure if needs key or not
  template<typename resendDataPeriod, typename WriterParams>
  struct SpdpParticipantWriter :
    StatelessWriter<resendDataPeriod, WriterParams, ReliabilityKind_t::best_effort,
    TopicKind_t::with_key>
  {
    // TODO Specific behavior here
    // Otherwise this could just be a alias.
  };

  template<typename ReaderParams>
  struct SpdpParticipantReader :
    StatelessReader<ReaderParams, ReliablityKind_t::best_effort, TopicKind_t::with_key>
  {
    // TODO
  };

}

#endif  // CMBML__SPDP___HPP_
