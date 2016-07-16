#include <cassert>
#include <cmbml/structure/writer.hpp>

using namespace cmbml;

CacheChange ReaderCacheAccessor::pop_next_requested_change() {
  assert(writer_cache);
  assert(writer_cache->contains_change(lowest_requested_seq_num));
  CacheChange change = writer_cache->copy_change(lowest_requested_seq_num);
  requested_seq_num_set.pop_front();
  lowest_requested_seq_num = requested_seq_num_set.front();
  assert(num_requested_changes > 0);
  --num_requested_changes;
  if (num_requested_changes == 0) {
    requested_changes_empty = true;
  }
  return change;
}

CacheChange ReaderCacheAccessor::pop_next_unsent_change() {
  uint64_t next_seq = (highest_seq_num_sent + 1).value();
  assert(writer_cache);
  assert(writer_cache->contains_change(next_seq));
  // Copy out the cachechange here
  CacheChange change = writer_cache->copy_change(next_seq);
  highest_seq_num_sent = change.sequence_number;
  assert(num_unsent_changes > 0);
  --num_unsent_changes;
  if (num_unsent_changes == 0) {
    unsent_changes_empty = true;
  }
  return change;
}

// request_seq_numbers must be sorted in ascending order
// Should this clear the set of requested changes first?
void ReaderCacheAccessor::set_requested_changes(
    const List<SequenceNumber_t> & request_seq_numbers)
{
  if (num_requested_changes == 0 && request_seq_numbers.size() > 0) {
    requested_changes_not_empty = true;
  }

  num_requested_changes += request_seq_numbers.size();

  for (const auto & seq : request_seq_numbers) {
    requested_seq_num_set.push_back(seq);
    // writer_cache[seq].status = requested;
  }
}

void ReaderLocator::reset_unsent_changes() {
  highest_seq_num_sent = writer_cache->get_min_sequence_number();
  num_unsent_changes = 0;
  unsent_changes_empty = true;
}

bool ReaderLocator::operator==(const ReaderLocator & loc) {
  return key_matches(loc.get_locator());
}

bool ReaderLocator::key_matches(const Locator_t & loc) {
  for (size_t i = 0; i < 16; ++i) {
    if (loc.address[i] != locator.address[i]) {
      return false;
    }
  }
  return locator.kind == loc.kind && locator.port == loc.port;
}

ChangeForReader ReaderProxy::pop_next_requested_change() {
  CacheChange change = ReaderCacheAccessor::pop_next_requested_change();
  return ChangeForReader(std::move(change));
}

ChangeForReader ReaderProxy::pop_next_unsent_change() {
  CacheChange change = ReaderCacheAccessor::pop_next_unsent_change();
  return ChangeForReader(std::move(change));
}

void ReaderProxy::set_requested_changes(List<SequenceNumber_t> & request_seq_numbers) {
  num_requested_changes = request_seq_numbers.size();
  ReaderCacheAccessor::set_requested_changes(request_seq_numbers);
}

void ReaderProxy::set_acked_changes(const SequenceNumber_t & seq_num) {
  if (seq_num > highest_acked_seq_num) {
    // Assume we acked sequentially
    num_unacked_changes = 0;
    unsent_changes_empty = true;
  }
  highest_acked_seq_num = seq_num;
}

bool ReaderProxy::operator==(const ReaderProxy & proxy) {
  return fields.remote_reader_guid == proxy.fields.remote_reader_guid;
}

bool ReaderProxy::key_matches(const GUID_t & guid) {
  return fields.remote_reader_guid == guid;
}

void ReaderProxy::add_change_for_reader(ChangeForReader && change) {
  // Naive?
  switch (change.status) {
    case ChangeForReaderStatus::unsent:
      if (num_unsent_changes == 0) {
        unsent_changes_not_empty = true;
      }
      ++num_unsent_changes;
      break;
    case ChangeForReaderStatus::unacknowledged:
      if (num_unacked_changes == 0) {
        unacked_changes_not_empty = true;
      }
      ++num_unacked_changes;
      break;
    case ChangeForReaderStatus::requested:
      if (num_requested_changes == 0) {
        requested_changes_not_empty = true;
      }
      ++num_requested_changes;
      break;
    default:
      break;
      // Status is of a type we don't track, so don't worry about it
  }

  writer_cache->add_change(std::move(change));
}
