#ifndef CMBML__READER_STATE_MACHINE__HPP_
#define CMBML__READER_STATE_MACHINE__HPP_

#include <boost/msm-lite.hpp>

#include <cmbml/reader.hpp>
#include <cmbml/types.hpp>
#include <cmbml/history.hpp>
#include <cmbml/data.hpp>

namespace cmbml {

  auto on_reader_created = [](auto e) {
    WriterProxy writer_proxy(e.remote_writer_guid, e.unicast_locators, e.multicast_locators);
    e.reader.add_matched_writer(std::move(writer_proxy));
    // WriterProxy is initialized with all past and future samples from the Writer as discussed in 8.4.10.4.
  };

  auto on_reader_deleted = [](auto e) {
    e.reader.remove_matched_writer(e.writer);
  };

  auto on_data_received_stateless = [](auto e) {
    CacheChange change(e.data);
    e.reader.reader_cache.add_change(std::move(change));
  };

  // TODO
  auto on_heartbeat = [](auto e) {
    // the_writer_proxy.missing_changes_update(HEARTBEAT.lastSN);
    // the_writer_proxy.lost_changes_update(HEARTBEAT.firstSN);
  };

  // TODO
  auto on_data = [](auto e) {
    CacheChange change(e.data);
    e.reader.reader_cache.add_change(std::move(change));
    // the_writer_proxy.received_change_set(a_change.sequenceNumber);
  };

  // TODO
  auto on_gap = [](auto e) {
    // FOREACH seq_num IN [GAP.gapStart, GAP.gapList.base-1] DO {
    // the_writer_proxy.irrelevant_change_set(seq_num);
    // }
    // FOREACH seq_num IN GAP.gapList DO {
    // the_writer_proxy.irrelevant_change_set(seq_num);
    // }
  };

  // TODO
  auto on_heartbeat_response_delay = [](auto e) {
    // missing_seq_num_set.base := the_writer_proxy.available_changes_max() + 1;
    // missing_seq_num_set.set := <empty>;
    // FOREACH change IN the_writer_proxy.missing_changes() DO
    // ADD change.sequenceNumber TO missing_seq_num_set.set;
    // send ACKNACK(missing_seq_num_set);
    // TODO see spec page  127 for special capacity handling behavior
  };

  // TODO! Receiver source???
  auto on_data_received_stateful = [](auto e) {
    // a_change := new CacheChange(DATA);
    CacheChange change(e.data);
    // writer_guid := {Receiver.SourceGuidPrefix, DATA.writerId};
    // writer_proxy := the_rtps_reader.matched_writer_lookup(writer_guid);
    // expected_seq_num := writer_proxy.available_changes_max() + 1;
    // if ( a_change.sequenceNumber >= expected_seq_num ) {
    // the_rtps_reader.reader_cache.add_change(a_change);
    // writer_proxy.received_change_set(a_change.sequenceNumber);
    // if ( a_change.sequenceNumber > expected_seq_num ) {
    // writer_proxy.lost_changes_update(a_change.sequenceNumber);
    // }
    // }
    // After the transition the following post-conditions hold:
    // writer_proxy.available_changes_max() >= a_change.sequenceNumber
  };

  template<typename ReaderT>
  struct ReaderMsm {
    struct reader_created {
      ReaderT & reader;
      GUID_t & remote_writer_guid;
      List<Locator_t> & unicast_locators;
      List<Locator_t> & multicast_locators;
    };

    struct data_received {
      ReaderT & reader;
      SerializedData & data;
    };

    struct data_received_reliable {
      ReaderT & reader;
      SerializedData & data;
      WriterProxy & proxy;
    };

    struct reader_deleted {
      ReaderT & reader;
      WriterProxy * writer;
    };

    // struct writer_matched {};

    // Need to make sure this transition is encoded with the right precedence
    struct heartbeat_not_final_flag {};
    struct heartbeat_not_liveliness_flag {};
    struct heartbeat_received {
      WriterProxy & writer;
      Heartbeat & heartbeat;
    };

    struct heartbeat_response_delay {
      WriterProxy & writer;
    };

    struct missing_changes_empty {};
    struct missing_changes_not_empty {};

    struct gap_received {
      WriterProxy & writer;
      Gap & gap;
    };
    // struct writer_unmatched {};

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
        *initial_s + event<reader_created>                             = waiting_s,
        waiting_s  + event<data_received> / on_data_received_stateless = waiting_s,
        waiting_s  + event<reader_deleted>                             = final_s
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

      state<class initial> initial_s;
      state<class waiting> waiting_s;
      state<class final_> final_s;

      return boost::msm::lite::make_transition_table(
        *initial_s + event<reader_created> / on_reader_created         = waiting_s,
        waiting_s  + event<data_received>  / on_data_received_stateful = waiting_s,
        waiting_s  + event<reader_deleted> / on_reader_deleted         = final_s
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

      // TODO
      return boost::msm::lite::make_transition_table(
        *initial_s  + event<reader_created> / on_reader_created = waiting_s,

        waiting_s   + event<heartbeat_not_final_flag>           = must_ack_s,
        waiting_s   + event<heartbeat_not_liveliness_flag>      = may_ack_s,
        waiting_s   + event<heartbeat_received>                 = waiting_s,

        may_ack_s   + event<missing_changes_empty>                                  = waiting_s,
        may_ack_s   + event<missing_changes_not_empty>                              = must_ack_s,
        must_ack_s  + event<heartbeat_response_delay> / on_heartbeat_response_delay = waiting_s,

        *initial2_s + event<reader_created>                    = ready_s,
        // TODO Are these states needed?
        // ready_s     + event<heartbeat_not_final_flag> /  = ready_s,
        // ready_s     + event<heartbeat_not_liveliness_flag> / [](){} = ready_s,
        ready_s     + event<heartbeat_received>     / on_heartbeat = ready_s,
        ready_s     + event<data_received_reliable> / on_data      = ready_s,
        ready_s     + event<gap_received>           / on_gap       = ready_s,

        *initial_s  + event<reader_deleted> / on_reader_deleted = final_s,
        waiting_s   + event<reader_deleted> / on_reader_deleted = final_s,
        may_ack_s   + event<reader_deleted> / on_reader_deleted = final_s,
        must_ack_s  + event<reader_deleted> / on_reader_deleted = final_s,
        *initial2_s + event<reader_deleted> / on_reader_deleted = final_s,
        ready_s     + event<reader_deleted> / on_reader_deleted = final_s
      );
    }

  };

}

#endif  // CMBML__READER_STATE_MACHINE__HPP_
