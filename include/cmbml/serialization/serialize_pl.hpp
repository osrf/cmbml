#ifndef CMBML__SERIALIZE_PL__HPP_
#define CMBML__SERIALIZE_PL__HPP_

namespace cmbml {

  template<typename DstT, typename SrcT>
  StatusCode deserialize_pl(DstT & dst, const SrcT & src, size_t i) {
    return StatusCode::ok;
  }

}

#endif  // CMBML__SERIALIZE_PL__HPP_
