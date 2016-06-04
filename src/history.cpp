#include <cmbml/history.hpp>

using namespace cmbml;

void HistoryCache::addChange(CacheChange * change) {
}
void HistoryCache::removeChange(CacheChange * change) {
}
SequenceNumber_t HistoryCache::get_min_sequence_number() const {
  return {0, 0};
}
SequenceNumber_t HistoryCache::get_max_sequence_number() const {
  return {0, 0};
}

