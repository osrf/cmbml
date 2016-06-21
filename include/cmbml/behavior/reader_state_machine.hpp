#ifndef CMBML__READER_STATE_MACHINE__HPP_
#define CMBML__READER_STATE_MACHINE__HPP_

#include <boost/msm-lite.hpp>

#include <cmbml/behavior/reader_state_machine_actions.hpp>
#include <cmbml/behavior/reader_state_machine_events.hpp>

namespace cmbml {


  template<typename ReaderT>
  struct ReaderMsm {
    // Best-effort Stateless Reader specialization
    template<bool Stateful,
      ReliabilityKind_t reliabilityLevel,
      typename std::enable_if<
        reliabilityLevel == ReliabilityKind_t::best_effort &&
        !Stateful
      >::type * = nullptr>
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

    // TODO better error checking for this invalid specialization
    /*
    template<bool Stateful,
    ReliabilityKind_t reliabilityLevel,
    typename std::enable_if<
      reliabilityLevel == ReliabilityKind_t::reliable &&
      !Stateful
    >::type * = nullptr>
    auto configure() {
    }
    */


    template<bool Stateful,
    ReliabilityKind_t reliabilityLevel,
    typename std::enable_if<
      reliabilityLevel == ReliabilityKind_t::best_effort &&
      Stateful
    >::type * = nullptr>
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      using namespace cmbml::reader_events;

      state<class initial> initial_s;
      state<class waiting> waiting_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<reader_created<ReaderT>> / on_reader_created         = waiting_s,
        waiting_s  + event<data_received<ReaderT>>  / on_data_received_stateful = waiting_s,
        waiting_s  + event<reader_deleted<ReaderT>> / on_reader_deleted         = final_s
      );
    }

    template<bool Stateful,
    ReliabilityKind_t reliabilityLevel,
    typename std::enable_if<
      reliabilityLevel == ReliabilityKind_t::reliable &&
      Stateful
    >::type * = nullptr>
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      using namespace cmbml::reader_events;

      state<class initial> initial_s;
      state<class waiting> waiting_s;
      state<class may_ack> may_ack_s;
      state<class must_ack> must_ack_s;
      state<class initial2> initial2_s;
      state<class ready> ready_s;
      state<class final_> final_s;

      // TODO
      return boost::msm::lite::make_transition_table(
        *initial_s  + event<reader_created<ReaderT>> / on_reader_created = waiting_s,

        waiting_s   + event<heartbeat_not_final_flag>           = must_ack_s,
        waiting_s   + event<heartbeat_not_liveliness_flag>      = may_ack_s,
        waiting_s   + event<heartbeat_received>                 = waiting_s,

        may_ack_s   + event<missing_changes_empty>              = waiting_s,
        may_ack_s   + event<missing_changes_not_empty>          = must_ack_s,
        must_ack_s  + event<heartbeat_response_delay> / on_heartbeat_response_delay = waiting_s,

        *initial2_s + event<reader_created<ReaderT>>                    = ready_s,
        // TODO Are these states needed?
        // ready_s     + event<heartbeat_not_final_flag> /  = ready_s,
        // ready_s     + event<heartbeat_not_liveliness_flag> / [](){} = ready_s,
        ready_s     + event<heartbeat_received>     / on_heartbeat = ready_s,
        ready_s     + event<data_received_reliable<ReaderT>> / on_data      = ready_s,
        ready_s     + event<gap_received>           / on_gap       = ready_s,

        *initial_s  + event<reader_deleted<ReaderT>> / on_reader_deleted = final_s,
        waiting_s   + event<reader_deleted<ReaderT>> / on_reader_deleted = final_s,
        may_ack_s   + event<reader_deleted<ReaderT>> / on_reader_deleted = final_s,
        must_ack_s  + event<reader_deleted<ReaderT>> / on_reader_deleted = final_s,
        *initial2_s + event<reader_deleted<ReaderT>> / on_reader_deleted = final_s,
        ready_s     + event<reader_deleted<ReaderT>> / on_reader_deleted = final_s
      );
    }

  };

}

#endif  // CMBML__READER_STATE_MACHINE__HPP_
