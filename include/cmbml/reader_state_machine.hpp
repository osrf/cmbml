#ifndef CMBML__READER_STATE_MACHINE__HPP_
#define CMBML__READER_STATE_MACHINE__HPP_

#include <boost/msm-lite.hpp>

#include <cmbml/types.hpp>

namespace cmbml {

struct ReaderMsm {
  struct reader_created {};
  struct data_received {};
  struct reader_deleted {};


  struct reader_matched {};
  // Need to make sure this transition is encoded with the right precedence
  struct heartbeat_not_final_flag {};
  struct heartbeat_not_liveliness_flag {};
  struct heartbeat_received {};
  struct heartbeat_response_delay {};

  struct missing_changes_empty {};
  struct missing_changes_not_empty {};
  struct gap_received {};
  struct reader_unmatched {};

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

    state<class initial> initial_s;
    state<class waiting> waiting_s;
    state<class final_> final_s;

    return boost::msm::lite::make_transition_table(
      *initial_s + event<reader_created> / [](){} = waiting_s,
      waiting_s  + event<data_received> /  [](){} = waiting_s,
      waiting_s  + event<reader_deleted> /  [](){} = final_s
    );
  }

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

    state<class initial> initial_s;
    state<class waiting> waiting_s;
    state<class final_> final_s;

    // Table is basically the same, side effects are different
    // TODO: Can this be combined with the above specialization and the side effects
    // selected in a different way?
    return boost::msm::lite::make_transition_table(
      *initial_s + event<reader_created> / [](){} = waiting_s,
      waiting_s  + event<data_received> /  [](){} = waiting_s,
      waiting_s  + event<reader_deleted> /  [](){} = final_s
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

    state<class initial> initial_s;
    state<class waiting> waiting_s;
    state<class may_ack> may_ack_s;
    state<class must_ack> must_ack_s;
    state<class initial2> initial2_s;
    state<class ready> ready_s;
    state<class final_> final_s;

    return boost::msm::lite::make_transition_table(
      *initial_s + event<reader_matched> / [](){} = waiting_s,

      waiting_s  + event<heartbeat_not_final_flag> /  [](){} = must_ack_s,
      waiting_s  + event<heartbeat_not_liveliness_flag> /  [](){} = may_ack_s,
      waiting_s  + event<heartbeat_received> /  [](){} = waiting_s,

      may_ack_s  + event<missing_changes_empty> /  [](){} = waiting_s,
      may_ack_s  + event<missing_changes_not_empty> /  [](){} = must_ack_s,
      must_ack_s  + event<heartbeat_response_delay> /  [](){} = waiting_s,
      *initial2_s  + event<reader_matched> / [](){} = ready_s,
      ready_s + event<heartbeat_not_final_flag> / [](){} = ready_s,
      ready_s + event<heartbeat_not_liveliness_flag> / [](){} = ready_s,
      ready_s + event<heartbeat_received> / [](){} = ready_s,
      ready_s + event<data_received> / [](){} = ready_s,
      ready_s + event<gap_received> / [](){} = ready_s,

      initial_s  + event<reader_unmatched> /  [](){} = final_s,
      waiting_s  + event<reader_unmatched> /  [](){} = final_s,
      may_ack_s  + event<reader_unmatched> /  [](){} = final_s,
      must_ack_s  + event<reader_unmatched> /  [](){} = final_s,
      initial2_s  + event<reader_unmatched> /  [](){} = final_s,
      ready_s  + event<reader_unmatched> /  [](){} = final_s
    );
  }

};

}

#endif  // CMBML__READER_STATE_MACHINE__HPP_
