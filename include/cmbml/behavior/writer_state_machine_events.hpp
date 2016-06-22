#ifndef CMBML__WRITER_STATE_MACHINE_EVENTS__HPP_
#define CMBML__WRITER_STATE_MACHINE_EVENTS__HPP_

#include <cmbml/types.hpp>
#include <cmbml/message/message_receiver.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/structure/writer.hpp>

namespace cmbml {

namespace stateless_writer {
  template<typename StatelessWriterT>
  struct configured_locator {
    StatelessWriterT & writer;
    ReaderLocator && locator;
  };
  struct unsent_changes {};
  struct unsent_changes_empty {};

  template<typename StatelessWriterT>
  struct can_send {
    StatelessWriterT & writer;
    ReaderLocator & locator;
    bool writer_has_key;
  };

  template<typename StatelessWriterT>
  struct released_locator {
    StatelessWriterT & writer;
    ReaderLocator * locator;
  };

  template<typename StatelessWriterT>
  struct after_heartbeat {
    StatelessWriterT & writer;
    ReaderLocator & locator;
  };

  template<typename StatelessWriterT>
  struct acknack_received {
    StatelessWriterT & writer;
    MessageReceiver & receiver;
    AckNack & acknack;
  };
  struct requested_changes {};
  struct requested_changes_empty {};
  struct after_nack_delay {};
}  // namespace stateless_writer

namespace stateful_writer {
  template<typename StatefulWriterT>
  struct configured_reader {
    StatefulWriterT & writer;
    GUID_t & reader_guid;
    bool expects_inline_qos;
    List<Locator_t> & unicast_locator_list;
    List<Locator_t> & multicast_locator_list;
  };

  template<typename StatefulWriterT>
  struct released_reader {
    StatefulWriterT & writer;
    ReaderProxy * reader;
  };

  struct can_send {
    ReaderProxy & reader_proxy;
    bool writer_has_key;  // TODO writer_has_key everywhere can be compile-time
  };

  template<typename StatefulWriterT>
  struct new_change {
    StatefulWriterT & writer;
    ReaderProxy & reader_proxy;
    ChangeForReader change;
  };

  struct unsent_changes {};
  struct unsent_changes_empty {};

  struct removed_change {
    ChangeForReader & change;
  };

  struct unacked_changes_empty {};
  struct unacked_changes {};

  template<typename StatefulWriterT>
  struct after_heartbeat {
    StatefulWriterT & writer;
    // XXX Not sure if ReaderProxy is needed here
    ReaderProxy & reader_proxy;
  };

  template<typename StatefulWriterT>
  struct acknack_received {
    StatefulWriterT & writer;
    ReaderProxy & reader_proxy;
    AckNack & acknack;
  };

  struct requested_changes_empty {};
  struct requested_changes {};
  struct after_nack_delay {};

}  // namespace stateful_writer

}


#endif  // CMBML__WRITER_STATE_MACHINE_EVENTS__HPP_
