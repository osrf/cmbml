#ifndef CMBML__DATA__HPP_
#define CMBML__DATA__HPP_

#include <cmbml/history.hpp>
#include <cmbml/types.hpp>

namespace cmbml {

// CDR is the only serialization mechanism we need so this probably doesn't need to be generic.
struct SerializedData {

  // Construct a serialized message from a cache change.
  SerializedData(const CacheChange & change, bool expects_inline_qos) {
    // TODO
  }
};

struct Heartbeat {
  Heartbeat(
    const GUID_t & writer_guid,
    const SequenceNumber_t & seq_num_min,
    const SequenceNumber_t & seq_num_max)
  {
    // TODO
  }
};

struct Gap {

};

}

#endif  // CMBML__DATA__HPP_
