#include <cmbml/cmbml.hpp>

using namespace cmbml;

int main(int argc, char ** argv) {
  // TODO Think about the instantiation patterns and easier defaults
  // Depending on the application requirements, deployment configuration and underlying transports, the end-user may want
  // to tune the timing characteristics of the RTPS protocol.
  // Therefore, where the requirements on the protocol behavior allow delayed responses or specify periodic events,
  // implementations must allow the end-user to tune those timing characteristics.
  SpdpDiscoData data;
  Participant p(data);


  StatelessWriter<true,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateless_writer(p);

  StatelessWriter<true,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateless_writer(p);

  StatefulWriter<true,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateful_writer(p);

  StatefulWriter<true,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateful_writer(p);

  StatelessReader<false,
    EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
      best_effort_stateless_reader(p);
  StatefulReader<true, EndpointParams<ReliabilityKind_t::best_effort, TopicKind_t::with_key>>
    best_effort_stateful_reader(p);

  StatefulReader<true,
    EndpointParams<ReliabilityKind_t::reliable, TopicKind_t::with_key>>
      reliable_stateful_reader(p);

  SyncExecutor & exec = SyncExecutor::get_instance();
  udp::Transport transport;
  {
    dds::DataReader<SpdpDiscoData, decltype(best_effort_stateless_reader)> reader(p);
    reader.add_tasks(transport, exec);
  }

  {
    dds::DataReader<SpdpDiscoData, decltype(best_effort_stateful_reader)> reader(p);
    reader.add_tasks(transport, exec);
  }

  {
    dds::DataReader<SpdpDiscoData, decltype(reliable_stateful_reader)> reader(p);
    reader.add_tasks(transport, exec);
  }

  {
    dds::DataWriter<SpdpDiscoData, decltype(best_effort_stateless_writer)> writer(p);
    writer.add_tasks(transport, exec);
  }

  {
    dds::DataWriter<SpdpDiscoData, decltype(reliable_stateless_writer)> writer(p);
    writer.add_tasks(transport, exec);
  }

  {
    dds::DataWriter<SpdpDiscoData, decltype(best_effort_stateful_writer)> writer(p);
    writer.add_tasks(transport, exec);
  }

  {
    dds::DataWriter<SpdpDiscoData, decltype(reliable_stateful_writer)> writer(p);
    writer.add_tasks(transport, exec);
  }

  return 0;
}
