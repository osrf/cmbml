#ifndef CMBML__WRITER_STATE_MACHINE__HPP_
#define CMBML__WRITER_STATE_MACHINE__HPP_

#include <boost/msm-lite.hpp>

#include <cmbml/types.hpp>
#include <cmbml/structure/writer.hpp>

namespace cmbml {

namespace stateless_writer {

  auto on_configured_locator = [](auto & e) {
    e.writer.add_reader_locator(std::move(e.locator));
  };

  // TODO
  auto on_can_send = [](auto e) {
    // pseudocode from the spec:
    //   a_change := the_reader_locator.next_unsent_change();
    //   DATA = new DATA(a_change);
    //   IF (the_reader_locator.expectsInlineQos) {
    //   DATA.inlineQos := the_writer.related_dds_writer.qos;
    //   }
    //   DATA.readerId := ENTITYID_UNKNOWN;
    //   sendto the_reader_locator.locator, DATA;
    CacheChange next_change = e.locator.pop_next_unsent_change();
    // TODO Fill out fields of Data here or in constructor
    Data data(next_change, e.locator.expects_inline_qos);
    e.locator.send(data);

    // Postcondition:
    // change is removed from 
    //   ( a_change BELONGS-TO the_reader_locator.unsent_changes() ) == FALSE
  };

  // TODO
  auto on_can_send_reliable = [](auto e) {
    // a_change := the_reader_locator.next_requested_change();
    // IF a_change IN the_writer.writer_cache.changes {
    // DATA = new DATA(a_change);
    // IF (the_reader_locator.expectsInlineQos) {
    // DATA.inlineQos := the_writer.related_dds_writer.qos;
    // }
    // DATA.readerId := ENTITYID_UNKNOWN;
    // sendto the_reader_locator.locator, DATA;
    // }
    // ELSE {
    // GAP = new GAP(a_change.sequenceNumber);
    // GAP.readerId := ENTITYID_UNKNOWN;
    // sendto the_reader_locator.locator, GAP;
    // }
  };

  auto on_released_locator = [](auto e) {
    // we are moving the ReaderLocator out of the writer.
    // will we need a better way to refer to it? an iterator perhaps?
    e.writer.remove_reader_locator(e.locator);
  };

  auto on_heartbeat = [](auto e) {
    const SequenceNumber_t & seq_min = e.writer.writer_cache.get_min_sequence_number();
    const SequenceNumber_t & seq_max = e.writer.writer_cache.get_max_sequence_number();
    // seq_num_min := the_rtps_writer.writer_cache.get_seq_num_min();
    // seq_num_max := the_rtps_writer.writer_cache.get_seq_num_max();
    Heartbeat heartbeat(e.writer.guid, seq_min, seq_max);
    // HEARTBEAT := new HEARTBEAT(the_rtps_writer.writerGuid, seq_num_min, seq_num_max);
    // HEARTBEAT.FinalFlag := SET;
    // HEARTBEAT.readerId := ENTITYID_UNKNOWN;
    // sendto the_reader_locator, HEARTBEAT;
    e.locator.send(heartbeat);
  };


  // TODO
  auto on_acknack = [](auto e) {
    // FOREACH reply_locator_t IN { Receiver.unicastReplyLocatorList,
    // Receiver.multicastReplyLocatorList }
    // reader_locator := the_rtps_writer.reader_locator_lookup(reply_locator_t);
    // reader_locator.requested_changes_set(ACKNACK.readerSNState.set);
  };

  // StatelessWriter state machine

  // Needs readerLocator construction parameters from the discover protocol
  template<typename StatelessWriterT>
  struct StatelessWriterMsm {
    // TODO can the stateful and stateless writer share the event transition structs?
    struct configured_locator {
      StatelessWriterT & writer;
      ReaderLocator && locator;
    };
    struct unsent_changes {};
    struct unsent_changes_empty {};
    struct can_send {
      StatelessWriterT & writer;
      ReaderLocator & locator;
    };
    struct released_locator {
      StatelessWriterT & writer;
      ReaderLocator * locator;
    };

    struct after_heartbeat {
      StatelessWriterT & writer;
      ReaderLocator & locator;
    };
    struct acknack_received {};
    struct requested_changes {};
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
        *initial_s + event<configured_locator>   / on_configured_locator = idle_s,
        idle_s     + event<unsent_changes>                               = pushing_s,
        pushing_s  + event<unsent_changes_empty>                         = idle_s,
        pushing_s  + event<can_send>             / on_can_send           = pushing_s,

        // TODO use orthogonal region instead                           
        *initial_s  + event<released_locator>    / on_released_locator   = final_s,
        idle_s      + event<released_locator>    / on_released_locator   = final_s,
        pushing_s   + event<released_locator>    / on_released_locator   = final_s
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
        *initial_s   + event<configured_locator>     / on_configured_locator = announcing_s,
        announcing_s + event<unsent_changes>                                 = pushing_s,
        pushing_s    + event<unsent_changes_empty>                           = announcing_s,
        pushing_s    + event<can_send>               / on_can_send           = pushing_s,
        announcing_s + event<after_heartbeat>        / on_heartbeat          = announcing_s,
        waiting_s    + event<acknack_received>       / on_acknack            = waiting_s,
        waiting_s    + event<requested_changes>                              = must_repair_s,
        must_repair_s + event<acknack_received>      / on_acknack            = must_repair_s,
        must_repair_s + event<after_nack_delay>                              = repairing_s,
        repairing_s + event<can_send>                / on_can_send_reliable  = repairing_s,
        repairing_s + event<requested_changes_empty>                         = waiting_s,

        // use orthogonal region instead
        *initial_s    + event<released_locator>      / on_released_locator   = final_s,
        announcing_s  + event<released_locator>      / on_released_locator   = final_s,
        pushing_s     + event<released_locator>      / on_released_locator   = final_s,
        waiting_s     + event<released_locator>      / on_released_locator   = final_s,
        must_repair_s + event<released_locator>      / on_released_locator   = final_s,
        repairing_s   + event<released_locator>      / on_released_locator   = final_s
      );
    }
  };

}  // namespace stateless_writer


namespace stateful_writer {


  auto on_configured_reader = [](auto e) {
    ReaderProxy reader(
        e.reader_guid, e.expects_inline_qos, e.unicast_locator_list, e.multicast_locator_list);
    e.writer.add_matched_reader(std::move(reader));
  };

  auto on_released_reader = [](auto e) {
    e.writer.remove_matched_reader(e.reader);
  };

  // major TODO
  auto on_can_send = [](auto e) {
    // a_change := the_reader_proxy.next_unsent_change();
    // a_change.status := UNDERWAY;
    // if (a_change.is_relevant) {
    // DATA = new DATA(a_change);
    // IF (the_reader_proxy.expectsInlineQos) {
    // DATA.inlineQos := the_rtps_writer.related_dds_writer.qos;
    // }
    // DATA.readerId := ENTITYID_UNKNOWN;
    // send DATA;
    // }
    CacheChange change = e.reader_proxy.pop_next_unsent_change();
    // Data data(change);
  };

  // TODO
  auto on_new_change = [](auto e) {
    // ADD a_change TO the_reader_proxy.changes_for_reader;
    // IF (DDS_FILTER(the_reader_proxy, change)) THEN change.is_relevant := FALSE;
    // ELSE change.is_relevant := TRUE;
    // IF (the_rtps_writer.pushMode == true) THEN change.status := UNSENT;
    // ELSE change.status := UNACKNOWLEDGED;
  };

  // Sure more side effects than this are needed?
  auto on_removed_change = [](auto e) {
    e.change.is_relevant = false;
  };

  // TODO
  auto on_heartbeat = [](auto e) {
    const SequenceNumber_t & seq_min = e.writer.writer_cache.get_min_sequence_number();
    const SequenceNumber_t & seq_max = e.writer.writer_cache.get_max_sequence_number();
    Heartbeat heartbeat(e.writer.guid, seq_min, seq_max);

    // HEARTBEAT := new HEARTBEAT(the_rtps_writer.writerGuid, seq_num_min, seq_num_max);
    // HEARTBEAT.FinalFlag := NOT_SET;
    // HEARTBEAT.readerId := ENTITYID_UNKNOWN;
    // send HEARTBEAT;
  };

  // TODO!!!
  auto on_acknack = [](auto e) {
    // the_rtps_writer.acked_changes_set(ACKNACK.readerSNState.base - 1);
    // the_reader_proxy.requested_changes_set(ACKNACK.readerSNState.set);
    //
    // Postconditions:
    //   MIN { change.sequenceNumber IN the_reader_proxy.unacked_changes() } >=
    //   ACKNACK.readerSNState.base - 1

  };

  // TODO same as on_can_send but without setting ENTITYID_UNKNOWN
  auto on_can_send_repairing = [](auto e) {
    // a_change := the_reader_proxy.next_requested_change();
    // a_change.status := UNDERWAY;
    // if (a_change.is_relevant) {
    // DATA = new DATA(a_change, the_reader_proxy.remoteReaderGuid);
    // IF (the_reader_proxy.expectsInlineQos) {
    // DATA.inlineQos := the_rtps_writer.related_dds_writer.qos;
    // }
    // send DATA;
    // }
    // else {
    // GAP = new GAP(a_change.sequenceNumber, the_reader_proxy.remoteReaderGuid);
    // send GAP;
    // }
    //
    // postcondition:
    //  ( a_change BELONGS-TO the_reader_proxy.requested_changes() ) == FALSE
  };


  template<typename StatefulWriterT>
  struct StatefulWriterMsm {

    struct configured_reader {
      StatefulWriterT & writer;
      GUID_t & reader_guid;
      bool expects_inline_qos;
      List<Locator_t> & unicast_locator_list;
      List<Locator_t> & multicast_locator_list;
    };

    struct released_reader {
      StatefulWriterT & writer;
      ReaderProxy * reader;
    };

    struct can_send {
      ReaderProxy & reader_proxy;
    };

    struct new_change {
      StatefulWriterT & writer;
      ReaderProxy & reader_proxy;
    };

    struct unsent_changes {};
    struct unsent_changes_empty {};

    struct removed_change {
      // CacheChange & change;
      ChangeForReader & change;
    };

    struct unacked_changes_empty {};
    struct unacked_changes {};

    struct after_heartbeat {
      StatefulWriterT & writer;
    };
    struct acknack_received {
      StatefulWriterT & writer;
      ReaderProxy & reader_proxy;
    };
    struct requested_changes_empty {};
    struct requested_changes {};
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
        *initial_s + event<configured_reader>    / on_configured_reader = idle_s,
        idle_s     + event<unsent_changes>                              = pushing_s,
        pushing_s  + event<unsent_changes_empty>                        = idle_s,
        pushing_s  + event<can_send>             / on_can_send          = pushing_s,
        *ready_s   + event<new_change>           / on_new_change        = ready_s,

        *initial_s + event<released_reader>      / on_released_reader = final_s,
        idle_s     + event<released_reader>      / on_released_reader = final_s,
        pushing_s  + event<released_reader>      / on_released_reader = final_s,
        *ready_s   + event<released_reader>      / on_released_reader = final_s
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
        *initial_s    + event<configured_reader>     / on_configured_reader = announcing_s,
        announcing_s  + event<unsent_changes>                               = pushing_s,
        pushing_s     + event<unsent_changes_empty>                         = announcing_s,
        pushing_s     + event<can_send>              / on_can_send          = pushing_s,
        announcing_s  + event<unacked_changes_empty>                        = idle_s,
        idle_s        + event<unacked_changes>                              = announcing_s,
        // TODO I think this on_heartbeat (T7) is the same between ReliableWriters
        announcing_s  + event<after_heartbeat>       / on_heartbeat         = announcing_s,
        waiting_s     + event<acknack_received>      / on_acknack           = waiting_s,
        waiting_s     + event<requested_changes>                            = must_repair_s,
        must_repair_s + event<acknack_received>      / on_acknack           = must_repair_s,
        must_repair_s + event<after_nack_delay>                             = repairing_s,
        // TODO on_send is SLIGHTLY different here! don't send an unknown ReaderID
        repairing_s   + event<can_send>             / on_can_send_repairing = repairing_s,
        repairing_s   + event<requested_changes_empty>                      = waiting_s,

        *ready_s      + event<new_change>     / on_new_change     = ready_s,
        *ready_s      + event<removed_change> / on_removed_change = ready_s,

        *initial_s    + event<released_reader> / on_released_reader = final_s,
        idle_s        + event<released_reader> / on_released_reader = final_s,
        pushing_s     + event<released_reader> / on_released_reader = final_s,
        *ready_s      + event<released_reader> / on_released_reader = final_s,
        announcing_s  + event<released_reader> / on_released_reader = final_s,
        waiting_s     + event<released_reader> / on_released_reader = final_s,
        must_repair_s + event<released_reader> / on_released_reader = final_s,
        repairing_s   + event<released_reader> / on_released_reader = final_s
      );
    }

  };

}  // namespace stateful_writer
}

#endif  // CMBML__WRITER_STATE_MACHINE__HPP_
