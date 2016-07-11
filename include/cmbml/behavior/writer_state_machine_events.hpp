#ifndef CMBML__WRITER_STATE_MACHINE_EVENTS__HPP_
#define CMBML__WRITER_STATE_MACHINE_EVENTS__HPP_

#include <cmbml/types.hpp>
#include <cmbml/message/message_receiver.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/structure/writer.hpp>

namespace cmbml {

  template<typename StatelessWriterT>
  struct configured_locator {
    StatelessWriterT & writer;
    ReaderLocator && locator;
  };
  struct unsent_changes {};
  struct unsent_changes_empty {};

  template<typename WriterT, typename Transport = udp::Context>
  struct can_send {
    WriterT & writer;
    Transport & context;
  };

  template<typename StatelessWriterT>
  struct released_locator {
    StatelessWriterT & writer;
    ReaderLocator * locator;
  };

  template<typename WriterT,
    typename Transport = udp::Context>
  struct after_heartbeat {
    WriterT & writer;
    Transport & context;
  };

  template<typename WriterT>
  struct acknack_received {
    WriterT & writer;
    AckNack && acknack;
    MessageReceiver & receiver;
  };

  struct requested_changes {};
  struct requested_changes_empty {};
  struct after_nack_delay {};

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

  template<typename StatefulWriterT>
  struct new_change {
    StatefulWriterT & writer;
    ReaderProxy & reader_proxy;
    ChangeForReader change;
  };

  struct removed_change {
    ChangeForReader & change;
  };

  struct unacked_changes_empty {};
  struct unacked_changes {};


}


#endif  // CMBML__WRITER_STATE_MACHINE_EVENTS__HPP_
