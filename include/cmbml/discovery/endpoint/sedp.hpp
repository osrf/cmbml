#ifndef CMBML__SEDP___HPP_
#define CMBML__SEDP___HPP_

namespace cmbml {

  // TODO Set defaults based on spec plug 'n play parameters
  // Initialize default locators
  // Template on the PSM type?
  template<typename resendDataPeriod, typename WriterParams>
  struct SedpPubWriter :
    StatefulWriter<resendDataPeriod, WriterParams, ReliabilityKind_t::reliable,
    TopicKind_t::with_key>
  {
  };

  template<typename ReaderParams>
  struct SedpPubReader :
    StatefulReader<ReaderParams, ReliablityKind_t::reliable, TopicKind_t::with_key>
  {
  };

  template<typename resendDataPeriod, typename WriterParams>
  struct SedpSubWriter :
    StatefulWriter<resendDataPeriod, WriterParams, ReliabilityKind_t::reliable,
    TopicKind_t::with_key>
  {
  };

  template<typename ReaderParams>
  struct SedpSubReader :
    StatefulReader<ReaderParams, ReliablityKind_t::reliable, TopicKind_t::with_key>
  {
  };

  template<typename resendDataPeriod, typename WriterParams>
  struct SedpTopicsWriter :
    StatefulWriter<resendDataPeriod, WriterParams, ReliabilityKind_t::reliable,
    TopicKind_t::with_key>
  {
  };

  template<typename ReaderParams>
  struct SedpTopicsReader :
    StatefulReader<ReaderParams, ReliablityKind_t::reliable, TopicKind_t::with_key>
  {
  };

}

#endif  // CMBML__SEDP___HPP_
