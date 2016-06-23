#ifndef CMBML__HISTORY__HPP_
#define CMBML__HISTORY__HPP_

#include <map>

#include <cmbml/message/submessage.hpp>
#include <cmbml/types.hpp>

namespace cmbml {
  enum class ChangeKind_t {
    alive, not_alive_disposed, not_alive_unregistered
  };
  using InstanceHandle_t = std::array<Octet, 16>;

  struct Data;
  struct CacheChange {
    explicit CacheChange(const Data & data);
    CacheChange(const CacheChange &) = default;
    CacheChange(CacheChange &&) = default;
    ChangeKind_t kind;
    GUID_t writer_guid;
    InstanceHandle_t instance_handle;
    SequenceNumber_t sequence_number;

    // How to represent the type of this data in a generic way?
    // optional
    // if present, represents the serialized data stored in history
    // Data data_value;
    SerializedData data;
  };

  bool compare_cache_change_seq(
      const std::pair<uint64_t, CacheChange> & a,
      const std::pair<uint64_t, CacheChange> & b)
  {
    return a.first < b.first;
  }

  struct HistoryCache {
    // addChange should move the input cache change
    void add_change(CacheChange && change);
    CacheChange remove_change(const SequenceNumber_t & sequence_number);
    CacheChange remove_change(const uint64_t sequence_number);

    CacheChange copy_change(const SequenceNumber_t & sequence_number) const;
    CacheChange copy_change(const uint64_t sequence_number) const;

    bool contains_change(const SequenceNumber_t & seq_num) const;
    bool contains_change(uint64_t seq_num) const;
    const SequenceNumber_t & get_min_sequence_number() const;
    const SequenceNumber_t & get_max_sequence_number() const;
  private:
    std::map<uint64_t, CacheChange> changes;
    SequenceNumber_t min_seq = {INT32_MAX, INT32_MAX};
    SequenceNumber_t max_seq = {INT32_MIN, 0};
  };

  // TODO Again, I think inheritance is too expensive here and composition should be used instead!
  enum class ChangeForReaderStatus {
    unsent, unacknowledged, requested, acknowledged, underway
  };

  struct ChangeForReader : CacheChange {
    ChangeForReaderStatus status = ChangeForReaderStatus::unsent;
    bool is_relevant = true;

    ChangeForReader(CacheChange && change) : CacheChange(change) {
    }
  };

  enum class ChangeFromWriterStatus {
    lost, missing, received, unknown
  };

  // TODO Give copy conversions to and from CacheChange objects
  struct ChangeFromWriter : CacheChange {
    ChangeFromWriterStatus status;
    bool is_relevant;
  };
}

#endif  // CMBML__HISTORY__HPP_
