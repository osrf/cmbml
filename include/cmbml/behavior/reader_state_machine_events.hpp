#ifndef CMBML__READER_STATE_MACHINE_EVENTS__HPP_
#define CMBML__READER_STATE_MACHINE_EVENTS__HPP_

#include <cmbml/structure/reader.hpp>
#include <cmbml/types.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/message/message_receiver.hpp>

namespace cmbml {

namespace reader_events {

  template<typename ReaderT>
  struct reader_created {
    ReaderT & reader;
    WriterProxyPOD && pod;
    /*
    GUID_t & remote_writer_guid;
    List<Locator_t> & unicast_locators;
    List<Locator_t> & multicast_locators;
    */
  };

  // TODO May need to make another event to avoid providing an unnecessary ref to receiver
  template<typename ReaderT>
  struct data_received {
    ReaderT & reader;
    Data && data;
    MessageReceiver & receiver;
  };

  template<typename ReaderT>
  struct reader_deleted {
    ReaderT & reader;
    WriterProxy & writer;
  };

  template<typename ReaderT>
  struct heartbeat_received {
    ReaderT & reader;
    Heartbeat & heartbeat;
  };

  template<typename ReaderT, typename TransportT>
  struct heartbeat_response_delay {
    ReaderT & reader;
    TransportT & transport;
  };

  struct missing_changes_empty {};
  struct missing_changes_not_empty {};

  template<typename ReaderT>
  struct gap_received {
    ReaderT & reader;
    Gap & gap;
  };
}

}

#endif  // CMBML__READER_STATE_MACHINE_EVENTS__HPP_
