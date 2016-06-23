#include <cassert>
#include <cmbml/structure/writer.hpp>

using namespace cmbml;

CacheChange ReaderCacheAccessor::pop_next_requested_change() {
  assert(writer_cache);
  assert(writer_cache->contains_change(lowest_requested_seq_num));
  CacheChange change = writer_cache->copy_change(lowest_requested_seq_num);
  requested_seq_num_set.pop_front();
  lowest_requested_seq_num = requested_seq_num_set.front();
  return change;
}

CacheChange ReaderCacheAccessor::pop_next_unsent_change() {
  uint64_t next_seq = (highest_seq_num_sent + 1).value();
  assert(writer_cache);
  assert(writer_cache->contains_change(next_seq));
  // Copy out the cachechange here
  CacheChange change = writer_cache->copy_change(next_seq);
  highest_seq_num_sent = change.sequence_number;
  return change;
}

// request_seq_numbers must be sorted in ascending order
void ReaderCacheAccessor::set_requested_changes(const List<SequenceNumber_t> & request_seq_numbers) {
  for (const auto & seq : request_seq_numbers) {
    requested_seq_num_set.push_back(seq);
    // writer_cache[seq].status = requested;
  }
}

void ReaderLocator::reset_unsent_changes() {
  highest_seq_num_sent = writer_cache->get_min_sequence_number();
}

bool ReaderLocator::locator_compare(const Locator_t & loc) {
  for (size_t i = 0; i < 16; ++i) {
    if (loc.address[i] != locator.address[i]) {
      return false;
    }
  }
  return locator.kind == loc.kind && locator.port == loc.port;
}

ChangeForReader ReaderProxy::pop_next_requested_change() {
  CacheChange change = cache_accessor.pop_next_requested_change();
  return ChangeForReader(std::move(change));
}

ChangeForReader ReaderProxy::pop_next_unsent_change() {
  CacheChange change = cache_accessor.pop_next_unsent_change();
  return ChangeForReader(std::move(change));
}

void ReaderProxy::set_requested_changes(List<SequenceNumber_t> & request_seq_numbers) {
  cache_accessor.set_requested_changes(request_seq_numbers);
}

void ReaderProxy::add_change_for_reader(ChangeForReader && change) {
  // Naive?

  writer_cache->add_change(std::move(change));
}
