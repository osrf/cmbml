#include <cmbml/structure/history.hpp>

using namespace cmbml;

void HistoryCache::add_change(CacheChange && change) {
  changes.push_back(change);
}
void HistoryCache::remove_change(CacheChange * change) {
}

static SequenceNumber_t temp_default = {0, 0};
SequenceNumber_t & HistoryCache::get_min_sequence_number() const {
  return temp_default;
}
SequenceNumber_t & HistoryCache::get_max_sequence_number() const {
  return temp_default;
}

