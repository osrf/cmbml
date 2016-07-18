#include <cmbml/cmbml.hpp>
#include <cmbml/utility/console_print.hpp>

using namespace cmbml;
using namespace cmbml::dds;

namespace hana = boost::hana;

// Message definition
// TODO Wrap define/adapt macros in library-specific calls instead of exposing hana
struct Int32 {
  BOOST_HANA_DEFINE_STRUCT(Int32,
    (uint32_t, data)
  );
};

int main(int argc, char ** argv) {
  Domain & domain = Domain::get_instance();
  udp::Context context;
  // SyncExecutor & executor = SyncExecutor::get_instance();
  Participant & participant = domain.create_new_participant<SyncExecutor>(context);
  // TODO Block until discovery is over?

  // TODO Pass allocator to history cache through these maps
  // Could also make it a handle for Executor and Context types.
  constexpr auto writer_options_map = make_option_map(
    hana::make_pair(EndpointOptions::stateful, false),
    hana::make_pair(EndpointOptions::reliability, ReliabilityKind_t::best_effort),
    hana::make_pair(EndpointOptions::topic_kind, TopicKind_t::with_key),
    hana::make_pair(EndpointOptions::push_mode, true)
  );

  // This macro makes a using declaration
  // May want to use a type_c instead to avoid namespace pollution.
  CMBML__MAKE_WRITER_OPTIONS(WriterOptions, writer_options_map);

  // TODO Finish these interfaces and functions
  auto writer = domain.create_data_writer<Int32, WriterOptions, SyncExecutor>(
    "chatter", participant, context);

  constexpr auto reader_options_map = make_option_map(
    hana::make_pair(EndpointOptions::stateful, false),
    hana::make_pair(EndpointOptions::reliability, ReliabilityKind_t::best_effort),
    hana::make_pair(EndpointOptions::topic_kind, TopicKind_t::with_key),
    hana::make_pair(EndpointOptions::expects_inline_qos, true),
    hana::make_pair(EndpointOptions::transport, hana::type_c<udp::Context>)
  );

  CMBML__MAKE_READER_OPTIONS(ReaderOptions, reader_options_map);

  auto reader = domain.create_data_reader<Int32, ReaderOptions, SyncExecutor>(
    "chatter", participant, context);

  {
    auto error_code = writer.write(Int32{42}, context);
    if (error_code != StatusCode::ok) {
      const char * error_string = error_code_string(error_code);
      CMBML__PRINT("Write returned error code: %s\n", error_string);
      return -1;
    }
  }

  Waitset<SyncExecutor> waitset;
  auto & read_condition = reader.create_read_condition();
  waitset.attach_condition(read_condition);
  waitset.wait();
  if (!read_condition.get_trigger_value()) {
    CMBML__PRINT("Wait woke up, but the read condition was not triggered.\n");
    return -1;
  }

  Int32 response;

  {
    auto error_code = reader.take(response);
    if (error_code != StatusCode::ok) {
      const char * error_string = error_code_string(error_code);
      CMBML__PRINT("Take returned error code: %s\n", error_string);
      return -1;
    }
  }

  assert(response.data == 42);
  return 0;
}
