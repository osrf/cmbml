#ifndef CMBML__READER_STATE_MACHINE_ACTIONS__HPP_
#define CMBML__READER_STATE_MACHINE_ACTIONS__HPP_

#include <cmbml/structure/reader.hpp>
#include <cmbml/types.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/message/data.hpp>

namespace cmbml {

  auto on_reader_created = [](auto e) {
    WriterProxy writer_proxy(e.remote_writer_guid, e.unicast_locators, e.multicast_locators);
    e.reader.add_matched_writer(std::move(writer_proxy));
    // TODO WriterProxy is initialized with all past and future samples from the Writer
    // as discussed in 8.4.10.4.
  };

  auto on_reader_deleted = [](auto e) {
    e.reader.remove_matched_writer(e.writer);
  };

  auto on_data_received_stateless = [](auto e) {
    CacheChange change(e.data);
    e.reader.reader_cache.add_change(std::move(change));
  };

  auto on_heartbeat = [](auto e) {
    e.writer.update_missing_changes(e.heartbeat.last_sn);
    e.writer.update_lost_changes(e.heartbeat.first_sn);
  };

  auto on_data = [](auto e) {
    CacheChange change(e.data);
    e.reader.reader_cache.add_change(std::move(change));
    e.proxy.set_received_change(change.sequence_number);
  };

  auto on_gap = [](auto e) {
    // Does the spec describe a range or a pair? Look at other impls
    for (const auto & seq_num : {e.gap.gap_start.value(), e.gap.gap_list.base.value() - 1}) {
      e.writer.set_irrelevant_change(seq_num);
    }
    for (const auto & seq_num : e.gap.gap_list.set) {
      e.writer.set_irrelevant_change(seq_num.value());
    }
  };

  auto on_heartbeat_response_delay = [](auto e) {
    SequenceNumberSet missing_seq_num_set({e.writer.max_available_changes() + 1});
    assert(missing_seq_num_set.set.empty());
    for (const auto & change : e.writer.missing_changes()) {
      missing_seq_num_set.set.push_back(change.sequence_number);
    }
    // TODO Implement send and the right constructor for ACKNACK
    // send ACKNACK(missing_seq_num_set);
    // TODO see spec page  127 for special capacity handling behavior
  };

  // TODO! Receiver source???
  auto on_data_received_stateful = [](auto e) {
    CacheChange change(e.data);
    // writer_guid := {Receiver.SourceGuidPrefix, DATA.writerId};
    GUID_t writer_guid = {e.receiver.source_guid_prefix, e.data.writer_id};
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

}
#endif  // CMBML__READER_STATE_MACHINE_ACTIONS__HPP_
