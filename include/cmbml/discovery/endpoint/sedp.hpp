#ifndef CMBML__SEDP___HPP_
#define CMBML__SEDP___HPP_

#include <cmbml/dds/reader.hpp>
#include <cmbml/dds/writer.hpp>

namespace cmbml {


  // TODO Set defaults based on spec plug 'n play parameters

  // TODO Set resend_data_period in option map?
  constexpr auto sedp_writer_options = hana::make_map(
    hana::make_pair(EndpointOptions::stateful, true),
    hana::make_pair(EndpointOptions::reliability, ReliablityKind_t::reliable),
    hana::make_pair(EndpointOptions::topic_kind, ReliablityKind_t::with_key),
    hana::make_pair(EndpointOptions::push_mode, true),
  );

  CMBML__MAKE_WRITER_OPTIONS(SEDPWriterOptions, sedp_writer_options);

  constexpr auto sedp_reader_options = hana::make_map(
    hana::make_pair(EndpointOptions::stateful, true),
    hana::make_pair(EndpointOptions::reliability, ReliablityKind_t::reliable),
    hana::make_pair(EndpointOptions::topic_kind, ReliablityKind_t::with_key),
    hana::make_pair(EndpointOptions::expects_inline_qos, false)
  );

  CMBML__MAKE_READER_OPTIONS(SEDPReaderOptions, sedp_reader_options);

  // TODO

  class SedpPubWriter :
    public dds::DataWriter<DiscoWriterData, SEDPWriterOptions>
  {
  };

  class SedpPubReader :
    public dds::DataReader<PDiscoWriterData, SEDPReaderOptions>
  {
  };

  class SedpSubWriter :
    public dds::DataWriter<DiscoReaderData, SEDPWriterOptions>
  {
  };

  class SedpSubReader :
    public dds::DataReader<DiscoReaderData, SEDPReaderOptions>
  {
  };

  class SedpTopicsWriter :
    public dds::DataWriter<TopicBuiltinTopicData, SEDPWriterOptions>
  {
  };

  class SedpTopicsReader :
    public dds::DataReader<TopicBuiltinTopicData, SEDPReaderOptions>
  {
  };

}

#endif  // CMBML__SEDP___HPP_
