#include <algorithm>
#include <cassert>
#include <cmbml/structure/history.hpp>

using namespace cmbml;

void HistoryCache::add_change(CacheChange && change) {
  min_seq = std::min(change.sequence_number, min_seq, SequenceNumber_t::less_than);
  max_seq = std::max(change.sequence_number, max_seq, SequenceNumber_t::less_than);
  changes.emplace(std::make_pair(change.sequence_number.value(), std::move(change)));
}

CacheChange HistoryCache::remove_change(const int64_t seq) {
  assert(changes.count(seq) != 0);
  if (seq == min_seq.value()) {
    // recompute min_seq
    min_seq = std::min_element(
        changes.begin(), changes.end(), compare_cache_change_seq)->second.sequence_number;
  }
  if (seq == max_seq.value()) {
    max_seq = std::max_element(
        changes.begin(), changes.end(), compare_cache_change_seq)->second.sequence_number;
  }
  auto ret = std::move(changes.at(seq));
  changes.erase(seq);
  return ret;

}

// didn't we decide that this should have pop semantics at some point?
CacheChange HistoryCache::remove_change(const SequenceNumber_t & seq) {
  return remove_change(seq.value());
}

const SequenceNumber_t & HistoryCache::get_min_sequence_number() const {
  return min_seq;
}
const SequenceNumber_t & HistoryCache::get_max_sequence_number() const {
  return max_seq;
}

