#ifndef CMBML__READER_STATE_MACHINE_ACTIONS__HPP_
#define CMBML__READER_STATE_MACHINE_ACTIONS__HPP_

#include <cmbml/structure/reader.hpp>
#include <cmbml/types.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/message/data.hpp>

namespace cmbml {

  auto on_reader_created = [](auto & e) {
    WriterProxy writer_proxy(e.remote_writer_guid, e.unicast_locators, e.multicast_locators);
    e.reader.add_matched_writer(std::move(writer_proxy));
    // TODO WriterProxy is initialized with all past and future samples from the Writer
    // as discussed in 8.4.10.4.
  };

  auto on_reader_deleted = [](auto & e) {
    e.reader.remove_matched_writer(e.writer);
  };

  auto on_data_received_stateless = [](auto & e) {
    CacheChange change(std::move(e.data));
    e.reader.reader_cache.add_change(std::move(change));
  };

  auto on_heartbeat = [](auto & e) {
    e.reader.for_each_matched_writer(
      [&e](auto & writer) {
        writer.update_missing_changes(e.heartbeat.last_sn);
        writer.update_lost_changes(e.heartbeat.first_sn);
      }
    );
  };

  auto on_data = [](auto & e) {
    CacheChange change(e.data);
    e.reader.reader_cache.add_change(std::move(change));
    // TODO Check that this lookup is correct.
    GUID_t writer_guid = {e.receiver.source_guid_prefix, e.data.writer_id};
    WriterProxy * proxy = e.reader.matched_writer_lookup(writer_guid);
    // TODO: This could be a warning in production.
    assert(proxy);
    proxy->set_received_change(change.sequence_number);
  };

  auto on_gap = [](auto & e) {
    // Does the spec describe a range or a pair? Look at other impls
    // I think it's safe to say it's a pair
    e.reader.for_each_matched_writer(
      [&e](auto & writer) {
        for (const auto & seq_num : {e.gap.gap_start.value(), e.gap.gap_list.base.value() - 1}) {
          writer->set_irrelevant_change(seq_num);
        }
        for (const auto & seq_num : e.gap.gap_list.set) {
          writer->set_irrelevant_change(seq_num.value());
        }
      }
    );
  };

  auto on_heartbeat_response_delay = [](auto & e) {
    // for (const auto & writer : reader.get_matched_writers()) {
    e.reader.for_each_matched_writer(
      [&e](auto & writer) {
        SequenceNumberSet missing_seq_num_set({writer.max_available_changes() + 1});
        assert(missing_seq_num_set.set.empty());
        for (const auto & change : writer.missing_changes()) {
          missing_seq_num_set.set.push_back(change.sequence_number);
        }
        // TODO Implement send and maybe a constructor for acknack
        // TODO see spec page  127 for capacity handling behavior in sequence set
        AckNack acknack;
        acknack.reader_id = e.reader.entity_id;
        acknack.writer_id = writer.get_guid().entity_id;
        acknack.reader_sn_state = std::move(missing_seq_num_set);
        // Current setting final=1, which means we do not expect a response from the writer
        acknack.final_flag = 1;
        e.writer.send(std::move(acknack), e.transport_context);
      }
    );
  };

  auto on_data_received_stateful = [](auto & e) {
    CacheChange change(std::move(e.data));
    GUID_t writer_guid = {e.receiver.source_guid_prefix, e.data.writer_id};
    WriterProxy * writer_proxy = e.reader.matched_writer_lookup(writer_guid);
    assert(writer_proxy);
    // XXX: This is only needed for the assert at the end
    const SequenceNumber_t & seq = change.sequence_number;
    SequenceNumber_t expected_seq_num = writer_proxy->max_available_changes() + 1;
    if (change.sequence_number >= expected_seq_num) {
      writer_proxy->set_received_change(change.sequence_number);
      if (change.sequence_number > expected_seq_num) {
        writer_proxy->update_lost_changes(change.sequence_number);
      }
      e.reader.reader_cache.add_change(std::move(change));
    }
    assert(writer_proxy->max_available_changes() >= seq);
  };

  // Guards
  auto not_final_guard = [](auto & e) {
    return !e.heartbeat.final_flag;
  };

  // Small subtlety based on literal interpretation of precedence in spec
  auto not_live_guard = [](auto & e) {
    return e.heartbeat.final_flag && !e.heartbeat.liveliness_flag;
  };
}
#endif  // CMBML__READER_STATE_MACHINE_ACTIONS__HPP_
