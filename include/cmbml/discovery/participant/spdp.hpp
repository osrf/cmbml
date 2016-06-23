#ifndef CMBML__SPDP___HPP_
#define CMBML__SPDP___HPP_

#include <cmbml/structure/writer.hpp>
#include <cmbml/structure/reader.hpp>

#include <cmbml/discovery/participant/spdp_disco_data.hpp>

namespace cmbml {


  // TODO Set defaults based on spec plug 'n play parameters
  template<typename PSM = udp::Context,
    typename resendDataPeriod = PSM::default_resend_data_period,
    typename WriterParams>
  struct SpdpParticipantWriter :
    StatelessWriter<resendDataPeriod, WriterParams, ReliabilityKind_t::best_effort,
    TopicKind_t::with_key>
  {
    // Default constructor
    SpdpParticipantWriter() {
      // add_reader_locator();
    }

    // TODO Specific API calls here?
    // Initialize default locators
    // Template on the PSM type?
    // need to start a thread to periodically send the spdp disco data
  };

  template<typename ReaderParams>
  struct SpdpParticipantReader :
    StatelessReader<ReaderParams, ReliablityKind_t::best_effort, TopicKind_t::with_key>
  {
    // TODO
    // Initialize default locators and parameters
  };

}

#endif  // CMBML__SPDP___HPP_
