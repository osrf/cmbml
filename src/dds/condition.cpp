#include <chrono>
#include <cmbml/dds/condition.hpp>

// should be inline
std::atomic<uint64_t> global_condition_counter;

namespace cmbml {
namespace dds {

Condition::Condition() :
  condition_id(global_condition_counter.exchange(global_condition_counter+1)),
  trigger_value(false)
{
}

bool Condition::get_trigger_value() const {
  return trigger_value.load();
}

// Blocking call that waits until the guard condition is triggered
StatusCode Condition::wait_until_trigger() {
  std::unique_lock<std::mutex> lock(cv_mutex);
  cv.wait(lock, [this]() {return trigger_value.load();});
  return StatusCode::ok;
}

// Wait with timeout
StatusCode Condition::wait_until_trigger(const std::chrono::nanoseconds & timeout) {
  std::unique_lock<std::mutex> lock(cv_mutex);
  bool status = cv.wait_for(lock, timeout, [this]() { return trigger_value.load();});
  return status ? StatusCode::ok : StatusCode::timeout;
}

}  // namespace dds
}  // namespace cmbml
