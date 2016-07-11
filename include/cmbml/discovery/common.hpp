#ifndef CMBML__DISCOVERY_COMMON___HPP_
#define CMBML__DISCOVERY_COMMON___HPP_

namespace cmbml {

  struct BuiltinTopicKey_t {
    BOOST_HANA_DEFINE_STRUCT(BuiltinTopicKey_t,
      (std::array<uint32_t, 3>, value)
    );
  };


}  // namespace cmbml

#endif  // CMBML__DISCOVERY_COMMON___HPP_
