#ifndef CMBML__UTILITY__METAFUNCTIONS__HPP_
#define CMBML__UTILITY__METAFUNCTIONS__HPP_

namespace cmbml {

/* Compile-time ternary statement.
 * Unlike std::conditional, which conditionally selects between types, this metafunction
 * selects between non-type constexpr values based on a boolean condition. */
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

/* Limited static-if implementation. */
template<bool condition>
struct conditionally_execute;

template<>
struct conditionally_execute<true> {
  template<typename CallbackT, typename ...Args>
  static void call(CallbackT && callback, Args && ...args) {
    callback(std::forward<Args>(args)...);
  }
};

template<>
struct conditionally_execute<false> {
  template<typename CallbackT, typename ...Args>
  static void call(CallbackT &&, Args && ...) {
  }
};

}  // namespace cmbml

#endif  // CMBML__UTILITY__METAFUNCTIONS__HPP_
