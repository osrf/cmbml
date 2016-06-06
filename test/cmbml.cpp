#include <cmbml/cmbml.hpp>

using namespace cmbml;

int main(int argc, char ** argv) {
  // TODO defaults for timing parameters
  //const Duration_t placeholder_duration(1, 0);

  StatelessWriter<DurationT<1, 0>,
    WriterParams<true, DurationT<1, 0>>,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateless_writer;
  return 0;
}
