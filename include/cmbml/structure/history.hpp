#ifndef CMBML__HISTORY__HPP_
#define CMBML__HISTORY__HPP_

#include <cmbml/types.hpp>

namespace cmbml {
  enum class ChangeKind_t {
    alive, not_alive_disposed, not_alive_unregistered
  };
  using InstanceHandle_t = std::array<Octet, 16>;

  struct SequenceNumber_t {
    int32_t high;
    uint32_t low;
  };

  struct Data;
  struct CacheChange {
    CacheChange(const Data & data);
    ChangeKind_t kind;
    GUID_t writer_guid;
    InstanceHandle_t instance_handle;
    SequenceNumber_t sequence_number;

    // How to represent the type of this data in a generic way?
    // optional
    // if present, represents the serialized data stored in history
    // Data data_value;
  };

  struct HistoryCache {

    // addChange should move the input cache change
    void add_change(CacheChange && change);
    void remove_change(CacheChange * change);
    SequenceNumber_t & get_min_sequence_number() const;
    SequenceNumber_t & get_max_sequence_number() const;
  private:
    List<CacheChange> changes;
  };

}

#endif  // CMBML__HISTORY__HPP_
