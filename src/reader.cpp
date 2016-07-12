#include <cmbml/structure/reader.hpp>
#include <cassert>
#include <algorithm>

using namespace cmbml;

// TODO optimizations
SequenceNumber_t WriterProxy::max_available_changes() {
  SequenceNumber_t max_seq = maximum_sequence_number;
  for (auto it = changes_from_writer.begin(); it != changes_from_writer.end(); ++it) {
    if (it->second.status == ChangeFromWriterStatus::received ||
        it->second.status == ChangeFromWriterStatus::lost)
    {
      if (it->second.sequence_number > max_seq) {
        max_seq = it->second.sequence_number;
      }
    }
  }
  return max_seq;
}

void WriterProxy::set_irrelevant_change(const SequenceNumber_t & seq_num) {
  set_irrelevant_change(seq_num.value());
}

// Call this if the change's status is about to change
// Detect if it was previously 
void WriterProxy::update_missing_changes_count(
  const ChangeFromWriter & change, ChangeFromWriterStatus future_status)
{
  if (change.status == ChangeFromWriterStatus::missing &&
      future_status != ChangeFromWriterStatus::missing)
  {
    assert(num_missing_changes > 0);
    --num_missing_changes;
    if (num_missing_changes == 0) {
      missing_changes_empty = true;
    }
  } else if (change.status != ChangeFromWriterStatus::missing &&
      future_status == ChangeFromWriterStatus::missing)
  {
    ++num_missing_changes;
    if (num_missing_changes == 1) {
      missing_changes_not_empty = true;
    }
  }
}

void WriterProxy::set_irrelevant_change(const int64_t seq_num) {
  assert(changes_from_writer.count(seq_num));
  ChangeFromWriter & ref = changes_from_writer.at(seq_num);
  ref.is_relevant = false;
  update_missing_changes_count(ref, ChangeFromWriterStatus::received);
  ref.status = ChangeFromWriterStatus::received;
}

void WriterProxy::update_lost_changes(const SequenceNumber_t & first_available_seq_num) {
  std::for_each(changes_from_writer.begin(), changes_from_writer.end(),
    [this, first_available_seq_num](auto & change_pair) {
      ChangeFromWriter & change = change_pair.second;
      if ((change.status == ChangeFromWriterStatus::unknown ||
            change.status == ChangeFromWriterStatus::missing) &&
          (change.sequence_number < first_available_seq_num))
      {
        update_missing_changes_count(change, ChangeFromWriterStatus::lost);
        change.status = ChangeFromWriterStatus::lost;
      }
    }
  );
}

void WriterProxy::update_missing_changes(const SequenceNumber_t & last_available_seq_num) {
  std::for_each(changes_from_writer.begin(), changes_from_writer.end(),
    [this, last_available_seq_num](auto & change_pair) {
      ChangeFromWriter & change = change_pair.second;
      if (change.status == ChangeFromWriterStatus::unknown &&
          change.sequence_number <= last_available_seq_num)
      {
        update_missing_changes_count(change, ChangeFromWriterStatus::missing);
        change.status = ChangeFromWriterStatus::missing;
      }
    }
  );
}

void WriterProxy::set_received_change(const SequenceNumber_t & seq_num) {
  assert(changes_from_writer.count(seq_num.value()));
  ChangeFromWriter & change = changes_from_writer.at(seq_num.value());

  update_missing_changes_count(change, ChangeFromWriterStatus::received);
  change.status = ChangeFromWriterStatus::received;
}

const GUID_t & WriterProxy::get_guid() {
  return remote_writer_guid;
}
