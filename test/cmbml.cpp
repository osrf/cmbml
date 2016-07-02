#include <cmbml/cmbml.hpp>

using namespace cmbml;

int main(int argc, char ** argv) {
  // TODO Think about the instantiation patterns and easier defaults
  // Depending on the application requirements, deployment configuration and underlying transports, the end-user may want
  // to tune the timing characteristics of the RTPS protocol.
  // Therefore, where the requirements on the protocol behavior allow delayed responses or specify periodic events,
  // implementations must allow the end-user to tune those timing characteristics.

  StatelessWriter<true,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateless_writer;

  StatelessWriter<true,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateless_writer;

  StatefulWriter<true,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateful_writer;

  StatefulWriter<true,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateful_writer;

  StatelessReader<false,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateless_reader;

  StatefulReader<true,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateful_reader;

  StatefulReader<true,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateful_reader;

  SyncExecutor exec;
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
    writer.add_tasks(exec);
  }

  {
    dds::DataWriter<decltype(reliable_stateless_writer)> writer;
    writer.add_tasks(exec);
  }

  {
    dds::DataWriter<decltype(best_effort_stateful_writer)> writer;
    writer.add_tasks(exec);
  }

  {
    dds::DataWriter<decltype(reliable_stateful_writer)> writer;
    writer.add_tasks(exec);
  }

  return 0;
}
