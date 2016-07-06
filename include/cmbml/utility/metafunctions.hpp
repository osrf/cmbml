#ifndef CMBML__UTILITY__METAFUNCTIONS__HPP_
#define CMBML__UTILITY__METAFUNCTIONS__HPP_

namespace cmbml {

template<bool condition, typename T, T true_value, T false_value>
struct ternary {
  struct ternary_helper_true {
    static const T value = true_value;
  };
  struct ternary_helper_false {
    static const T value = false_value;
  };
  static const T value = std::conditional<
    condition, ternary_helper_true, ternary_helper_false>::type::value;
};

}  // namespace cmbml

#endif  // CMBML__UTILITY__METAFUNCTIONS__HPP_
