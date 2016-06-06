#ifndef CMBML__WRITER_STATE_MACHINE__HPP_
#define CMBML__WRITER_STATE_MACHINE__HPP_

#include <boost/msm-lite.hpp>

#include <cmbml/types.hpp>

namespace cmbml {


  // StatelessWriter state machine

  // Needs readerLocator construction parameters from the discover protocol
  struct StatelessWriterMsm {
    // TODO can the stateful and stateless writer share the event transition structs?
    struct writer_configure_locator {};
    struct unsent_changes_not_empty {};
    struct unsent_changes_empty {};
    struct can_send {};
    struct writer_release_locator {};

    struct after_heartbeat {};
    struct acknack_received {};
    struct requested_changes_not_empty {};
    struct requested_changes_empty {};
    struct after_nack_delay {};

    template<ReliabilityKind_t reliabilityLevel,
      typename std::enable_if<
        reliabilityLevel == ReliabilityKind_t::best_effort
      >::type * = nullptr>
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;

      state<class initial> initial_s;
      state<class idle> idle_s;
      state<class pushing> pushing_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<writer_configure_locator> / [](){  } = idle_s ,
        idle_s     + event<unsent_changes_not_empty> / [](){  } = pushing_s,
        pushing_s  + event<unsent_changes_empty>     / [](){  } = idle_s,
        pushing_s  + event<can_send>                 / [](){  } = pushing_s,

        *initial_s  + event<writer_release_locator>   / [](){  } = final_s,
        idle_s      + event<writer_release_locator>   / [](){  } = final_s,
        pushing_s   + event<writer_release_locator>   / [](){  } = final_s
      );
    }

    template<ReliabilityKind_t reliabilityLevel,
      typename std::enable_if<
        reliabilityLevel == ReliabilityKind_t::reliable
      >::type * = nullptr>
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;

      state<class initial> initial_s;
      state<class announcing> announcing_s;
      state<class pushing> pushing_s;
      state<class waiting> waiting_s;
      state<class must_repair> must_repair_s;
      state<class repairing> repairing_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s   + event<writer_configure_locator> / [](){  } = announcing_s,
        announcing_s + event<unsent_changes_not_empty> / [](){  } = pushing_s,
        pushing_s    + event<unsent_changes_empty> / [](){  } = announcing_s,
        pushing_s    + event<can_send> / [](){  } = pushing_s,
        announcing_s + event<after_heartbeat> / [](){  } = announcing_s,
        waiting_s    + event<acknack_received> / [](){  } = waiting_s,
        waiting_s    + event<requested_changes_not_empty> / [](){  } = must_repair_s,
        must_repair_s + event<acknack_received> / [](){} = must_repair_s,
        must_repair_s + event<after_nack_delay> / [](){} = repairing_s,
        repairing_s + event<can_send>  / [](){} = repairing_s,
        repairing_s + event<requested_changes_empty>  / [](){} = waiting_s,

        *initial_s    + event<writer_release_locator>   / [](){  } = final_s,
        announcing_s  + event<writer_release_locator>   / [](){  } = final_s,
        pushing_s     + event<writer_release_locator>   / [](){  } = final_s,
        waiting_s     + event<writer_release_locator>   / [](){  } = final_s,
        must_repair_s + event<writer_release_locator>   / [](){  } = final_s,
        repairing_s   + event<writer_release_locator>   / [](){  } = final_s
      );
    }
  };

  struct StatefulWriterMsm {
    struct writer_configure_reader {};
    struct unsent_changes_not_empty {};
    struct unsent_changes_empty {};
    struct can_send {};
    struct new_change {};
    struct removed_change {};
    struct writer_release_reader {};
    struct unacked_changes_empty {};
    struct unacked_changes_not_empty {};
    struct after_heartbeat {};
    struct acknack_received {};
    struct requested_changes_empty {};
    struct requested_changes_not_empty {};
    struct after_nack_delay {};

    template<ReliabilityKind_t reliabilityLevel,
      typename std::enable_if<
        reliabilityLevel == ReliabilityKind_t::best_effort
      >::type * = nullptr>
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      state<class initial> initial_s;
      state<class idle> idle_s;
      state<class pushing> pushing_s;
      state<class pushing> ready_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<writer_configure_reader> / [](){} = idle_s,
        idle_s + event<unsent_changes_not_empty> / [](){} = pushing_s,
        pushing_s + event<unsent_changes_empty> / [](){} = idle_s,
        pushing_s + event<can_send> / [](){} = pushing_s,
        *ready_s + event<new_change> / [](){} = ready_s,

        *initial_s + event<writer_release_reader> / [](){} = final_s,
        idle_s + event<writer_release_reader> / [](){} = final_s,
        pushing_s + event<writer_release_reader> / [](){} = final_s,
        *ready_s + event<writer_release_reader> / [](){} = final_s
      );
    }


    template<ReliabilityKind_t reliabilityLevel,
      typename std::enable_if<
        reliabilityLevel == ReliabilityKind_t::reliable
      >::type * = nullptr>
    auto configure() {
      using boost::msm::lite::state;
      using boost::msm::lite::event;
      state<class initial> initial_s;
      state<class idle> idle_s;
      state<class pushing> pushing_s;
      state<class announcing> announcing_s;
      state<class waiting> waiting_s;
      state<class must_repair> must_repair_s;
      state<class repairing> repairing_s;
      state<class pushing> ready_s;
      state<class final_> final_s;


      return boost::msm::lite::make_transition_table(
        *initial_s + event<writer_configure_reader> / [](){} = announcing_s,
        announcing_s + event<unsent_changes_not_empty> / [](){} = pushing_s,
        pushing_s + event<unsent_changes_empty> / [](){} = announcing_s,
        pushing_s + event<can_send> / [](){} = pushing_s,
        announcing_s + event<unacked_changes_empty> / [](){} = idle_s,
        idle_s + event<unacked_changes_not_empty>  / [](){} = announcing_s,
        announcing_s + event<after_heartbeat>  / [](){} = announcing_s,
        waiting_s + event<acknack_received>  / [](){} = waiting_s,
        waiting_s + event<requested_changes_not_empty>  / [](){} = must_repair_s,
        must_repair_s + event<acknack_received>  / [](){} = must_repair_s,
        must_repair_s + event<after_nack_delay>  / [](){} = repairing_s,
        repairing_s + event<can_send> / [](){} = repairing_s,
        repairing_s + event<requested_changes_empty> / [](){} = waiting_s,
        *ready_s + event<new_change> / [](){} = ready_s,
        *ready_s + event<removed_change> / [](){} = ready_s,

        *initial_s + event<writer_release_reader> / [](){} = final_s,
        idle_s + event<writer_release_reader> / [](){} = final_s,
        pushing_s + event<writer_release_reader> / [](){} = final_s,
        *ready_s + event<writer_release_reader> / [](){} = final_s,
        announcing_s + event<writer_release_reader> / [](){} = final_s,
        waiting_s + event<writer_release_reader> / [](){} = final_s,
        must_repair_s + event<writer_release_reader> / [](){} = final_s,
        repairing_s + event<writer_release_reader> / [](){} = final_s
      );
    }

  };
}

#endif  // CMBML__WRITER_STATE_MACHINE__HPP_
