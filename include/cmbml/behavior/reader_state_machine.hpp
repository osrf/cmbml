#ifndef CMBML__READER_STATE_MACHINE__HPP_
#define CMBML__READER_STATE_MACHINE__HPP_

#include <boost/msm-lite.hpp>

#include <cmbml/behavior/reader_state_machine_actions.hpp>
#include <cmbml/behavior/reader_state_machine_events.hpp>

namespace cmbml {

  template<typename ReaderT>
  struct BestEffortStatelessReaderMsm {
    // Best-effort Stateless Reader specialization
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      using namespace cmbml::reader_events;

      state<class initial> initial_s;
      state<class waiting> waiting_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<reader_created<ReaderT>>                             = waiting_s,
        waiting_s  + event<data_received<ReaderT>> / on_data_received_stateless = waiting_s,
        waiting_s  + event<reader_deleted<ReaderT>>                             = final_s
      );
    }
  };

  template<typename ReaderT>
  struct BestEffortStatefulReaderMsm {
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      using namespace cmbml::reader_events;

      state<class initial> initial_s;
      state<class waiting> waiting_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<reader_created<ReaderT>>  / on_reader_created         = waiting_s,
        waiting_s  + event<data_received<ReaderT>>   / on_data_received_stateful = waiting_s,
        *waiting_s  + event<reader_deleted<ReaderT>> / on_reader_deleted         = final_s
      );
    }
  };

  template<typename Reader, typename Transport>
  struct ReliableStatefulReaderMsm {
    using ReaderT = Reader;
    using TransportT = Transport;

    auto configure() {
      using boost::msm::lite::event;
      using boost::msm::lite::on_entry;
      using boost::msm::lite::state;
      using namespace cmbml::reader_events;

      state<class initial> initial_s;
      state<class waiting> waiting_s;
      state<class may_ack> may_ack_s;
      state<class must_ack> must_ack_s;
      state<class initial2> initial2_s;
      state<class ready> ready_s;
      state<class any> any_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s  + event<reader_created<Reader>> / on_reader_created = waiting_s,

        waiting_s   + event<heartbeat_received<Reader>> [not_final_guard] = must_ack_s,
        waiting_s   + event<heartbeat_received<Reader>> [not_live_guard]  = may_ack_s,
        waiting_s   + event<heartbeat_received<Reader>>                   = waiting_s,
        may_ack_s   + event<missing_changes_empty>                = waiting_s,
        may_ack_s   + event<missing_changes_not_empty>            = must_ack_s,
        must_ack_s + event<heartbeat_response_delay<Reader, Transport>>
          / on_heartbeat_response_delay = waiting_s,

        *initial2_s + event<reader_created<Reader>>               = ready_s,
        ready_s     + event<heartbeat_received<Reader>>     / on_heartbeat = ready_s,
        ready_s     + event<data_received<Reader>> / on_data      = ready_s,
        ready_s     + event<gap_received<Reader>>           / on_gap       = ready_s,

        *any_s  + event<reader_deleted<Reader>> / on_reader_deleted = final_s
      );
    }
  };

}

#endif  // CMBML__READER_STATE_MACHINE__HPP_
