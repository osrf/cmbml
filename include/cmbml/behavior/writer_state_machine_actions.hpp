#ifndef CMBML__WRITER_STATE_MACHINE_ACTIONS__HPP_
#define CMBML__WRITER_STATE_MACHINE_ACTIONS__HPP_

#include <cmbml/types.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/structure/writer.hpp>

#include <cmbml/behavior/writer_state_machine_events.hpp>

namespace cmbml {

namespace stateless_writer {
  auto on_configured_locator = [](auto & e) {
    for (auto & locator : e.fields.unicast_locator_list) {
      e.writer.emplace_matched_reader(
        std::forward<Locator_t>(locator), e.fields.expects_inline_qos, &e.writer.writer_cache);
    }
    for (auto & locator : e.fields.multicast_locator_list) {
      e.writer.emplace_matched_reader(
        std::forward<Locator_t>(locator), e.fields.expects_inline_qos, &e.writer.writer_cache);
    }
  };

  // Need to get locator from data id?
  // Small optimization: best effort can_send state does not need a ref. to the Writer
  auto on_can_send = [](auto & e) {
    e.writer.for_each_matched_reader(
      [&e](auto & reader_locator) {
        CacheChange next_change = reader_locator.pop_next_unsent_change();
        Data data(std::move(next_change), reader_locator.expects_inline_qos,
            std::decay_t<decltype(e.writer)>::topic_kind == TopicKind_t::with_key);
        data.reader_id = entity_id_unknown;
        Packet<> packet = e.writer.participant.serialize_with_header(data);
        e.context.unicast_send(reader_locator.get_locator(), packet.data(), packet.size());
      }
    );
  };

  auto on_can_send_reliable = [](auto & e) {
    e.writer.for_each_matched_reader(
      [&e](auto & reader_locator) {
        CacheChange change = reader_locator.pop_next_unsent_change();
        if (e.writer.writer_cache.contains_change(change.sequence_number)) {
          Data data(std::move(change), reader_locator.expects_inline_qos,
            std::decay_t<decltype(e.writer)>::topic_kind == TopicKind_t::with_key);
          // TODO: inline_qos; need to copy from related DDS writer
          data.reader_id = entity_id_unknown;
          Packet<> packet = e.writer.participant.serialize_with_header(data);
          e.context.unicast_send(reader_locator.get_locator(), packet.data(), packet.size());
        } else {
          Gap gap(entity_id_unknown, e.writer.guid.entity_id, change.sequence_number);
          Packet<> packet = e.writer.participant.serialize_with_header(gap);
          e.context.unicast_send(reader_locator.get_locator(), packet.data(), packet.size());
        }
      }
    );
  };

  auto on_released_locator = [](auto & e) {
    // we are moving the ReaderLocator out of the writer.
    // will we need a better way to refer to it? an iterator perhaps?
    e.writer.remove_matched_reader(e.locator);
  };

  auto on_heartbeat = [](auto & e) {
    const SequenceNumber_t & seq_min = e.writer.writer_cache.get_min_sequence_number();
    const SequenceNumber_t & seq_max = e.writer.writer_cache.get_max_sequence_number();
    Heartbeat heartbeat(e.writer.guid, seq_min, seq_max);
    heartbeat.final_flag = 1;
    heartbeat.reader_id = entity_id_unknown;
    heartbeat.count = e.writer.heartbeat_count++;
    e.writer.send_to_all_locators(heartbeat, e.context);
  };


  auto on_acknack = [](auto & e) {
    auto locator_lambda = [&e](Locator_t & locator) {
      ReaderLocator & reader_locator = e.writer.lookup_matched_reader(locator);
      reader_locator.set_requested_changes(e.acknack.reader_sn_state.set);
    };
    for (auto & reply_locator : e.receiver.unicast_reply_locator_list) {
      locator_lambda(reply_locator);
    }
    for (auto & reply_locator : e.receiver.multicast_reply_locator_list) {
      locator_lambda(reply_locator);
    }
  };

}  // namespace stateless_writer

namespace stateful_writer {

  auto on_configured_reader = [](auto & e) {
    e.writer.emplace_matched_reader(std::move(e.fields),
      &e.writer.writer_cache);
  };

  auto on_released_reader = [](auto & e) {
    e.writer.remove_matched_reader(e.reader);
  };

  auto on_can_send_stateful = [](auto & e) {
    e.writer.for_each_matched_reader(
      [&e](auto & reader_proxy) {
        ChangeForReader change = reader_proxy.pop_next_unsent_change();
        change.status = ChangeForReaderStatus::underway;
        if (change.is_relevant) {
          Data data(std::move(change), reader_proxy.fields.expects_inline_qos,
            std::decay_t<decltype(e.writer)>::topic_kind == TopicKind_t::with_key);
          // TODO inline QoS
          data.reader_id = entity_id_unknown;
          Packet<> packet = e.writer.participant.serialize_with_header(data);
          for (auto & locator : reader_proxy.fields.unicast_locator_list) {
            e.context.unicast_send(locator, packet.data(), packet.size());
          }
        }
      }
    );
  };

  auto on_new_change = [](auto & e) {
    // TODO: interface for DDS filtering
    // IF (DDS_FILTER(the_reader_proxy, change)) THEN change.is_relevant := FALSE;
    // ELSE change.is_relevant := TRUE;
    e.change.is_relevant = true;
    if (e.writer.push_mode) {
      e.change.status = ChangeForReaderStatus::unsent;
    } else {
      e.change.status = ChangeForReaderStatus::unacknowledged;
    }
    // I think it's ok to reorder this
    e.reader_proxy.add_change_for_reader(std::move(e.change));
  };

  // TODO Surely more side effects than this are needed? e.g. remove from ReaderProxy
  auto on_removed_change = [](auto & e) {
    e.change.is_relevant = false;
  };

  auto on_heartbeat = [](auto & e) {
    const SequenceNumber_t & seq_min = e.writer.writer_cache.get_min_sequence_number();
    const SequenceNumber_t & seq_max = e.writer.writer_cache.get_max_sequence_number();
    Heartbeat heartbeat(e.writer.guid, seq_min, seq_max);
    heartbeat.final_flag = 0;
    heartbeat.reader_id = entity_id_unknown;
    // Send the packet using a multicast socket
    heartbeat.count = e.writer.heartbeat_count++;
    e.writer.send_to_all_locators(heartbeat, e.context);
  };

  auto on_acknack = [](auto & e) {
    // Look up which ReaderProxy to set changes in
    // TODO: The GUID prefix should be provided from message deserialization
    GUID_t reader_guid = {e.receiver.source_guid_prefix, e.acknack.reader_id};
    ReaderProxy & proxy = e.writer.lookup_matched_reader(reader_guid);
    proxy.set_acked_changes(e.acknack.reader_sn_state.base - 1);
    proxy.set_requested_changes(e.acknack.reader_sn_state.set);
    // TODO assert postconditions
    // Postconditions:
    //   MIN { change.sequenceNumber IN the_reader_proxy.unacked_changes() } >=
    //   ACKNACK.readerSNState.base - 1
  };

  auto on_can_send_repairing = [](auto & e) {
    e.writer.for_each_matched_reader(
      [&e](auto & reader_proxy) {
        ChangeForReader change = reader_proxy.pop_next_requested_change();
        change.status = ChangeForReaderStatus::underway;
        if (change.is_relevant) {
          Data data(std::move(change), reader_proxy.fields.expects_inline_qos,
            std::decay_t<decltype(e.writer)>::topic_kind == TopicKind_t::with_key);
          // ??? Is this implied by the spec?
          data.reader_id = reader_proxy.fields.remote_reader_guid.entity_id;
          // TODO inline QoS
          Packet<> packet = e.writer.participant.serialize_with_header(data);
          for (auto & locator : reader_proxy.fields.unicast_locator_list) {
            e.context.unicast_send(locator, packet.data(), packet.size());
          }
          // e.reader_proxy.send(std::move(data), e.context);
          // e.writer.send(std::move(data), e.context);
        } else {
          // TODO
          Gap gap(reader_proxy.fields.remote_reader_guid.entity_id, change.writer_guid.entity_id,
              change.sequence_number);
          Packet<> packet = e.writer.participant.serialize_with_header(gap);
          for (auto & locator : reader_proxy.fields.unicast_locator_list) {
            e.context.unicast_send(locator, packet.data(), packet.size());
          }
          // e.reader_proxy.send(std::move(gap), e.context);
          // e.writer.send(std::move(gap), e.context);
        }

      }
    );
  };

}  // namespace stateful_writer

}

#endif  // CMBML__WRITER_STATE_MACHINE_ACTIONS__HPP_
