#include <cmbml/cmbml.hpp>

using namespace cmbml;

int main(int argc, char ** argv) {
  // TODO Think about the instantiation patterns and easier defaults
  // Depending on the application requirements, deployment configuration and underlying transports, the end-user may want
  // to tune the timing characteristics of the RTPS protocol.
  // Therefore, where the requirements on the protocol behavior allow delayed responses or specify periodic events,
  // implementations must allow the end-user to tune those timing characteristics.
  // So maybe my plan for constexpr EVERYTHING is misguided

  StatelessWriter<DurationT<1, 0>,
    WriterParams<true, DurationT<1, 0>>,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateless_writer;

  StatelessWriter<DurationT<1, 0>,
    WriterParams<true, DurationT<1, 0>>,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateless_writer;

  StatefulWriter<
    WriterParams<true, DurationT<1, 0>>,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateful_writer;

  StatefulWriter<
    WriterParams<true, DurationT<1, 0>>,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateful_writer;

  StatelessReader<ReaderParams<false>,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateless_reader;

  StatefulReader<ReaderParams<true>,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateful_reader;

  StatefulReader<ReaderParams<true>,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateful_reader;

  {
    dds::DataReader<decltype(best_effort_stateless_reader)> reader;
  }

  {
    dds::DataReader<decltype(best_effort_stateful_reader)> reader;
  }

  {
    dds::DataReader<decltype(reliable_stateful_reader)> reader;
  }

  {
    dds::DataWriter<decltype(best_effort_stateless_writer)> writer;
  }

  {
    dds::DataWriter<decltype(reliable_stateless_writer)> writer;
  }

  {
    dds::DataWriter<decltype(best_effort_stateful_writer)> writer;
  }

  {
    dds::DataWriter<decltype(reliable_stateful_writer)> writer;
  }

  return 0;
}
