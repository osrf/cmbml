#ifndef CMBML__CMBML__HPP_
#define CMBML__CMBML__HPP_

#include <cmbml/structure/writer.hpp>
#include <cmbml/structure/reader.hpp>

// TODO
#include <cmbml/behavior/writer_state_machine.hpp>
#include <cmbml/behavior/reader_state_machine.hpp>


namespace cmbml {

using stateless_writer::StatelessWriterMsm;
using stateful_writer::StatefulWriterMsm;

}

#endif  // CMBML__CMBML__HPP_
