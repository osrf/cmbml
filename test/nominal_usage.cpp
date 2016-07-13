#include <cmbml/cmbml.hpp>

using namespace cmbml;
using namespace cmbml::dds;

namespace hana = boost::hana;

// Message definition
// TODO Wrap define/adapt macros in library-specific calls
struct Int32 {
  BOOST_HANA_DEFINE_STRUCT(Int32,
    (uint32_t, data)
  );
};


int main(int argc, char ** argv) {

  Domain & domain = Domain::get_instance();
  udp::Context context;
  SyncExecutor & executor = SyncExecutor::get_instance();
  Participant & participant = domain.create_new_participant<SyncExecutor>(context);
  // TODO Block until discovery is over?

  constexpr auto writer_options = make_option_map(
    hana::make_pair(hana::type_c<EndpointOptions::stateful>, false),
    hana::make_pair(hana::type_c<EndpointOptions::reliability>, ReliabilityKind_t::best_effort),
    hana::make_pair(hana::type_c<EndpointOptions::topic_kind>, TopicKind_t::with_key),
    hana::make_pair(hana::type_c<EndpointOptions::push_mode>, true)
  );

  // TODO Finish these interfaces and functions
  auto writer = Domain::create_data_writer<Int32>(
      participant, writer_options, context, executor);
  /*

  auto reader = participant.create_data_reader<Int32>(reader_options);

  writer.write();

  Waitset waitset;
  waitset.attach_condition(reader.read_condition);
  reader.take();
  */
}
