#ifndef CMBML__SEDP___HPP_
#define CMBML__SEDP___HPP_

#include <cmbml/dds/reader.hpp>
#include <cmbml/dds/writer.hpp>
#include <cmbml/discovery/endpoint/messages.hpp>

namespace cmbml {


  // TODO Set defaults based on spec plug 'n play parameters

  // TODO Set resend_data_period in option map?
  constexpr auto sedp_writer_options = hana::make_map(
    hana::make_pair(EndpointOptions::stateful, true),
    hana::make_pair(EndpointOptions::reliability, ReliabilityKind_t::reliable),
    hana::make_pair(EndpointOptions::topic_kind, TopicKind_t::with_key),
    hana::make_pair(EndpointOptions::push_mode, true)
  );

  CMBML__MAKE_WRITER_OPTIONS(SEDPWriterOptions, sedp_writer_options);

  constexpr auto sedp_reader_options = hana::make_map(
    hana::make_pair(EndpointOptions::stateful, true),
    hana::make_pair(EndpointOptions::reliability, ReliabilityKind_t::reliable),
    hana::make_pair(EndpointOptions::topic_kind, TopicKind_t::with_key),
    hana::make_pair(EndpointOptions::expects_inline_qos, false),
    hana::make_pair(EndpointOptions::transport, hana::type_c<udp::Transport>)
  );

  CMBML__MAKE_READER_OPTIONS(SEDPReaderOptions, sedp_reader_options);

  // TODO

  // Need a Writer to WriterProxyPOD conversion (same for Reader)
  class SedpPubWriter :
    public dds::DataWriter<DiscoWriterData, SEDPWriterOptions>
  {
  public:
    static constexpr EntityId_t get_id(){
      return {{{0x0, 0x0, 0x3}}, EntityKind::builtin_writer_with_key};
    }
    static const BuiltinEndpointKind kind = BuiltinEndpointKind::publications_writer;

    using ParentType = dds::DataWriter<DiscoWriterData, SEDPWriterOptions>;
    SedpPubWriter(Participant & p) : ParentType("", p) {
      // TODO
      rtps_writer.guid.entity_id = get_id();
    }

  };

  class SedpPubReader :
    public dds::DataReader<DiscoWriterData, SEDPReaderOptions>
  {
  public:
    static constexpr EntityId_t get_id(){
      return {{{0x0, 0x0, 0x3}}, EntityKind::builtin_reader_with_key};
    }
    static const BuiltinEndpointKind kind = BuiltinEndpointKind::publications_reader;

    using ParentType = dds::DataReader<DiscoWriterData, SEDPReaderOptions>;
    SedpPubReader(Participant & p) : ParentType("", p) {
      // TODO
      rtps_reader.guid.entity_id = get_id();
    }
  };

  class SedpSubWriter :
    public dds::DataWriter<DiscoReaderData, SEDPWriterOptions>
  {
  public:
    static constexpr EntityId_t get_id(){
      return {{{0x0, 0x0, 0x4}}, EntityKind::builtin_writer_with_key};
    }
    static const BuiltinEndpointKind kind = BuiltinEndpointKind::subscriptions_writer;

    using ParentType = dds::DataWriter<DiscoReaderData, SEDPWriterOptions>;
    SedpSubWriter(Participant & p) : ParentType("", p) {
      // TODO
      rtps_writer.guid.entity_id = get_id();
    }
  };

  class SedpSubReader :
    public dds::DataReader<DiscoReaderData, SEDPReaderOptions>
  {
  public:
    static constexpr EntityId_t get_id(){
      return {{{0x0, 0x0, 0x4}}, EntityKind::builtin_reader_with_key};
    }
    static const BuiltinEndpointKind kind = BuiltinEndpointKind::subscriptions_reader;

    using ParentType = dds::DataReader<DiscoReaderData, SEDPReaderOptions>;
    SedpSubReader(Participant & p) : ParentType("", p) {
      // TODO
      rtps_reader.guid.entity_id = get_id();
    }
  };

  class SedpTopicsWriter :
    public dds::DataWriter<TopicBuiltinTopicData, SEDPWriterOptions>
  {
  public:
    static constexpr EntityId_t get_id(){
      return {{{0x0, 0x0, 0x2}}, EntityKind::builtin_writer_with_key};
    }
    static const BuiltinEndpointKind kind = BuiltinEndpointKind::topics_writer;
    using ParentType = dds::DataWriter<TopicBuiltinTopicData, SEDPWriterOptions>;
    SedpTopicsWriter(Participant & p) : ParentType("", p) {
      rtps_writer.guid.entity_id = get_id();
      // TODO
    }
  };

  class SedpTopicsReader :
    public dds::DataReader<TopicBuiltinTopicData, SEDPReaderOptions>
  {
  public:
    static constexpr EntityId_t get_id(){
      return {{{0x0, 0x0, 0x2}}, EntityKind::builtin_reader_with_key};
    }
    static const BuiltinEndpointKind kind = BuiltinEndpointKind::topics_reader;

    using ParentType = dds::DataReader<TopicBuiltinTopicData, SEDPReaderOptions>;
    SedpTopicsReader(Participant & p) : ParentType("", p) {
      rtps_reader.guid.entity_id = get_id();
      // TODO
    }
  };

}

#endif  // CMBML__SEDP___HPP_
