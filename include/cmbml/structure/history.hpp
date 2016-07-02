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
    CacheChange(ChangeKind_t k, Data && data, InstanceHandle_t && handle, const GUID_t & writer_guid);
    CacheChange(ChangeKind_t k, InstanceHandle_t && handle, const GUID_t & writer_guid);
    ChangeKind_t kind;
    GUID_t writer_guid;
    InstanceHandle_t instance_handle;
    SequenceNumber_t sequence_number = {0, 0};

    // How to represent the type of this data in a generic way?
    // optional/could be empty
    // if present, represents the serialized data stored in history
    // Data data_value;
    SerializedData data;
  };

  static bool compare_cache_change_seq(
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
    void clear();

    template<typename CallbackT,
      typename std::enable_if<
        std::is_same<typename std::result_of<CallbackT>::type, bool>::value>::type * = nullptr>
    List<CacheChange> get_filtered_cache_changes(CallbackT & callback) const {
      List<CacheChange> ret;
      for (const auto & pair : changes) {
        auto change = pair.second;
        if (callback(change)) {
          ret.push_back(change);
        }
      }
      return ret;
    }

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
