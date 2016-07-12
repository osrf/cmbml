#include <cmbml/cmbml.hpp>

using namespace cmbml;

using namespace cmbml::dds;

int main(int argc, char ** argv) {
  Domain & domain = Domain::get_instance();
  udp::Context context;
  Participant & participant = domain.create_new_participant<SyncExecutor>(context);
  // TODO Block until discovery is over?

  // TODO Finish these interfaces and functions
  /*
  participant.create_data_writer();

  participant.create_data_reader();

  writer.write();

  Waitset waitset;
  waitset.attach_condition(reader.read_condition);
  reader.take();
  */
}
