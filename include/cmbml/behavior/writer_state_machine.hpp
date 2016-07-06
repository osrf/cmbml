#ifndef CMBML__WRITER_STATE_MACHINE__HPP_
#define CMBML__WRITER_STATE_MACHINE__HPP_

#include <boost/msm-lite.hpp>
#include <boost/hana.hpp>

#include <cmbml/behavior/writer_state_machine_actions.hpp>
#include <cmbml/behavior/writer_state_machine_events.hpp>

namespace hana = boost::hana;

namespace cmbml {

  template<typename WriterT>
  struct BestEffortStatelessWriterMsm
  {
    // Best effort, stateless writer specialization
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      using namespace cmbml::stateless_writer;

      state<class initial> initial_s;
      state<class idle> idle_s;
      state<class pushing> pushing_s;
      state<class any> any_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<configured_locator<WriterT>> / on_configured_locator = idle_s,
        idle_s     + event<unsent_changes>                                      = pushing_s,
        pushing_s  + event<unsent_changes_empty>                                = idle_s,
        pushing_s  + event<can_send<WriterT>>           / on_can_send           = pushing_s,

        *any_s  + event<released_locator<WriterT>>  / on_released_locator   = final_s
      );
    }
  };

  template<typename WriterT>
  struct ReliableStatelessWriterMsm
  {
    // Reliable stateless writer
    // Needs readerLocator construction parameters from the discover protocol
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      using namespace cmbml::stateless_writer;

      state<class initial> initial_s;
      state<class announcing> announcing_s;
      state<class pushing> pushing_s;
      state<class waiting> waiting_s;
      state<class must_repair> must_repair_s;
      state<class repairing> repairing_s;
      state<class any> any_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s   + event<configured_locator<WriterT>> / on_configured_locator = announcing_s,
        announcing_s + event<unsent_changes>                                      = pushing_s,
        pushing_s    + event<unsent_changes_empty>                                = announcing_s,
        pushing_s    + event<can_send<WriterT>>           / on_can_send           = pushing_s,
        announcing_s + event<after_heartbeat<WriterT>>    / on_heartbeat          = announcing_s,
        waiting_s    + event<acknack_received<WriterT>>   / on_acknack            = waiting_s,
        waiting_s    + event<requested_changes>                                   = must_repair_s,
        must_repair_s + event<acknack_received<WriterT>>  / on_acknack            = must_repair_s,
        must_repair_s + event<after_nack_delay>                                   = repairing_s,
        repairing_s + event<can_send<WriterT>>            / on_can_send_reliable  = repairing_s,
        repairing_s + event<requested_changes_empty>                              = waiting_s,

        *any_s        + event<released_locator<WriterT>> / on_released_locator = final_s
      );
    }
  };

  template<typename WriterT>
  struct BestEffortStatefulWriterMsm
  {
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      using namespace cmbml::stateful_writer;

      state<class initial> initial_s;
      state<class idle> idle_s;
      state<class pushing> pushing_s;
      state<class pushing> ready_s;
      state<class any> any_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<configured_reader<WriterT>> / on_configured_reader = idle_s,
        idle_s     + event<unsent_changes>                                    = pushing_s,
        pushing_s  + event<unsent_changes_empty>                              = idle_s,
        pushing_s  + event<can_send_stateful<>>          / on_can_send          = pushing_s,
        *ready_s   + event<new_change<WriterT>>        / on_new_change        = ready_s,

        *any_s + event<released_reader<WriterT>>   / on_released_reader   = final_s
      );
    }
  };

  template<typename Writer>
  struct ReliableStatefulWriterMsm
  {
    using WriterT = Writer;

    auto configure() {
      using boost::msm::lite::event;
      using boost::msm::lite::on_entry;
      using boost::msm::lite::state;
      using namespace cmbml::stateful_writer;

      state<class initial> initial_s;
      state<class idle> idle_s;
      state<class pushing> pushing_s;
      state<class announcing> announcing_s;
      state<class waiting> waiting_s;
      state<class must_repair> must_repair_s;
      state<class repairing> repairing_s;
      state<class pushing> ready_s;
      state<class any> any_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s    + event<configured_reader<Writer>> / on_configured_reader  = announcing_s,
        announcing_s  + event<unsent_changes>                                    = pushing_s,
        pushing_s     + event<unsent_changes_empty>                              = announcing_s,
        pushing_s     + event<can_send_stateful<>>       / on_can_send           = pushing_s,
        announcing_s  + event<unacked_changes_empty>                             = idle_s,
        idle_s        + event<unacked_changes>                                   = announcing_s,
        announcing_s  + event<after_heartbeat<Writer>>   / on_heartbeat = announcing_s,
        waiting_s     + event<acknack_received<Writer>>  / on_acknack            = waiting_s,
        waiting_s     + event<requested_changes>                                 = must_repair_s,
        must_repair_s + event<acknack_received<Writer>>  / on_acknack            = must_repair_s,
        must_repair_s + event<after_nack_delay>                                  = repairing_s,
        repairing_s   + event<can_send_stateful<>>       / on_can_send_repairing = repairing_s,
        repairing_s   + event<requested_changes_empty>                           = waiting_s,

        *ready_s      + event<new_change<Writer>>        / on_new_change     = ready_s,
        *ready_s      + event<removed_change> / on_removed_change = ready_s,

        *any_s        + event<released_reader<Writer>> / on_released_reader = final_s
      );
    }
  };

}

#endif  // CMBML__WRITER_STATE_MACHINE__HPP_
