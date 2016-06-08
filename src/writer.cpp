#include <cmbml/writer.hpp>

using namespace cmbml;

template<typename T>
T pop_next(List<T> & list) {
  auto next = std::move(list.back());
  list.pop_back();
  return std::move(next);
}

CacheChange ReaderLocator::pop_next_requested_change() {
  return std::move(pop_next(requested_changes_list));
}

CacheChange ReaderLocator::pop_next_unsent_change() {
  return std::move(pop_next(unsent_changes_list));
}
List<CacheChange> * ReaderLocator::requested_changes() const {
  return nullptr;
}
void ReaderLocator::set_requested_changes(List<SequenceNumber_t> & request_seq_numbers) {
}
List<CacheChange> * ReaderLocator::unsent_changes() const {
  return nullptr;
}

SequenceNumber_t ReaderProxy::set_acked_changes() {
  return {0, 0};
}
CacheChange ReaderProxy::pop_next_requested_change() {
  return std::move(pop_next(requested_changes_list));
}
CacheChange ReaderProxy::pop_next_unsent_change() {
  return std::move(pop_next(unsent_changes_list));
}

void ReaderProxy::set_requested_changes(List<SequenceNumber_t> & request_seq_numbers) {
}
List<ChangeForReader> * ReaderProxy::unacked_changes() const {
  return nullptr;
}
