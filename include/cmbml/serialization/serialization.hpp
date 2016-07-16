#ifndef CMBML__SERIALIZATION__SERIALIZATION_HPP_
#define CMBML__SERIALIZATION__SERIALIZATION_HPP_

#include <cmbml/serialization/serialize_cdr.hpp>
#include <cmbml/serialization/deserialize_cdr.hpp>
#include <cmbml/serialization/serialize_pl.hpp>

namespace cmbml {

  // Encapsulation identifiers
  // TODO Read encapsulation identifier from packet and condition serialization scheme on that.
  enum struct EncapsulationId : uint16_t {
    cdr_be = 0x0,  // Big-endian
    cdr_le = 0x1,  // Little-endian
    pl_cdr_be = 0x2,  // Parameter-list big-endian
    pl_cdr_le = 0x3,  // Parameter-list little-endian
  };

  StatusCode serialize_payload() {
    // TODO
    return StatusCode::ok;
  }

  template<typename DstT, typename SrcT>
  StatusCode deserialize_payload(DstT & dst, const SrcT & src) {
    // Check the first 2 octets.
    size_t i = 0;
    // TODO big-endian vs. little-endian settings
    EncapsulationId encapsulation_id;
    deserialize(encapsulation_id, src, i);
    uint16_t options;
    deserialize(options, src, i);
    // Dispatch the encapsulation type.
    switch (encapsulation_id) {
      case EncapsulationId::cdr_be:
      case EncapsulationId::cdr_le:
        return deserialize(dst, src, i);
      case EncapsulationId::pl_cdr_be:
      case EncapsulationId::pl_cdr_le:
        return deserialize_pl(dst, src, i);
      default:
        return StatusCode::deserialize_failed;
    }

    return StatusCode::ok;
  }

}  // namespace cmbml

#endif  // CMBML__SERIALIZATION__SERIALIZATION_HPP_
