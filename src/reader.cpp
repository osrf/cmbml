#include <cmbml/structure/reader.hpp>
#include <cassert>
#include <algorithm>

using namespace cmbml;

const SequenceNumber_t & WriterProxy::max_available_changes() {
  const SequenceNumber_t * max_seq = &maximum_sequence_number;
  for (auto it = changes_from_writer.begin(); it != changes_from_writer.end(); ++it) {
    if (it->second.status == ChangeFromWriterStatus::received ||
        it->second.status == ChangeFromWriterStatus::lost)
    {
      if (it->second.sequence_number > *max_seq) {
        max_seq = &it->second.sequence_number;
      }
    }
  }
  return *max_seq;
}

void WriterProxy::set_irrelevant_change(const SequenceNumber_t & seq_num) {
  set_irrelevant_change(seq_num.value());
}

void WriterProxy::set_irrelevant_change(const int64_t seq_num) {
  assert(changes_from_writer.count(seq_num));
  ChangeFromWriter & ref = changes_from_writer.at(seq_num);
  ref.is_relevant = false;
  ref.status = ChangeFromWriterStatus::received;
}

void WriterProxy::update_lost_changes(const SequenceNumber_t & first_available_seq_num) {
  std::for_each(changes_from_writer.begin(), changes_from_writer.end(),
    [first_available_seq_num](auto & change_pair) {
      ChangeFromWriter & change = change_pair.second;
      if ((change.status == ChangeFromWriterStatus::unknown ||
            change.status == ChangeFromWriterStatus::missing) &&
          (change.sequence_number < first_available_seq_num))
      {
        change.status = ChangeFromWriterStatus::lost;
      }
    }
  );
}

void WriterProxy::update_missing_changes(const SequenceNumber_t & last_available_seq_num) {
  std::for_each(changes_from_writer.begin(), changes_from_writer.end(),
    [last_available_seq_num](auto & change_pair) {
      ChangeFromWriter & change = change_pair.second;
      if (change.status == ChangeFromWriterStatus::unknown &&
          change.sequence_number <= last_available_seq_num)
      {
        change.status = ChangeFromWriterStatus::missing;
      }
    }
  );
}

void WriterProxy::set_received_change(const SequenceNumber_t & seq_num) {
  assert(changes_from_writer.count(seq_num.value()));
  changes_from_writer.at(seq_num.value()).status = ChangeFromWriterStatus::received;
}

const GUID_t & WriterProxy::get_guid() {
  return remote_writer_guid;
}
