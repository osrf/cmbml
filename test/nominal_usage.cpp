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
  Participant & participant = domain.create_new_participant<SyncExecutor>(context);
  // TODO Block until discovery is over?

  // compile-time strings... don't want to incur a runtime overhead on these keys...
  // may prefer an enum
  /*
  auto writer_options = make_option_map(
    hana::pair{EndpointOptions::topic_type, hana::type_c<Int32>},
    hana::pair{EndpointOptions::stateful, false},
    hana::pair{EndpointOptions::reliability, ReliabilityKind_t::best_effort},
    hana::pair{EndpointOptions::topic_kind, TopicKind_t::with_key}
    hana::pair{EndpointOptions::push_mode, true}
  );
  */

  // TODO Finish these interfaces and functions
  /*
  auto & writer = participant.create_data_writer<Int32>(writer_options);

  auto & reader = participant.create_data_reader<Int32>(reader_options);

  writer.write();

  Waitset waitset;
  waitset.attach_condition(reader.read_condition);
  reader.take();
  */
}
