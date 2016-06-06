#include <cmbml/writer.hpp>

using namespace cmbml;

ChangeForReader * ReaderLocator::next_requested_change() const {
  return nullptr;
}
ChangeForReader * ReaderLocator::next_unsent_change() const {
  return nullptr;
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
ChangeForReader * ReaderProxy::next_requested_change() const {
  return nullptr;
}
ChangeForReader * ReaderProxy::next_unsent_change() const {
  return nullptr;
}
List<ChangeForReader> * ReaderProxy::unsent_changes() const {
  return nullptr;
}
List<ChangeForReader> * ReaderProxy::requested_changes() const {
  return nullptr;
}
void ReaderProxy::set_requested_changes(List<SequenceNumber_t> & request_seq_numbers) {
}
List<ChangeForReader> * ReaderProxy::unacked_changes() const {
  return nullptr;
}
