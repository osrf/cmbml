#ifndef CMBML__DDS__WAITSET_HPP_
#define CMBML__DDS__WAITSET_HPP_

#include <chrono>
#include <cmbml/dds/condition.hpp>

namespace cmbml {
namespace dds {


  template<typename Executor>
  class Waitset {
  public:
    // Specify timeout value here?
    void attach_condition(Condition & condition) {
      if (condition_task_id_map.count(condition.condition_id) != 0) {
        return;
      }
      if (condition.get_trigger_value() && Executor::get_instance().is_running()) {
        Executor::get_instance().interrupt();
        return;
      }

      // Add condition to the Executor
      size_t task_id = Executor::get_instance().add_task(true,
        [&condition](const std::chrono::nanoseconds & t){
          return condition.wait_until_trigger(t);
        }
      );
      condition_task_id_map[condition.condition_id] = task_id;
    }

    void detach_condition(Condition & condition) {
      // TODO
      if (condition_task_id_map.count(condition.condition_id) == 0) {
        return;
      }
      Executor::get_instance().remove_task(condition_task_id_map[condition.condition_id]);
    }

    // ummmm
    List<Condition> get_conditions();

    void wait(const std::chrono::nanoseconds & timeout = std::chrono::nanoseconds(-1)) {
      // TODO
      // Need to have one thread per condition variable to see which one comes in first?
      // Need to keep track of futures
      // executor would help determine this
      List<size_t> task_ids;
      std::transform(condition_task_id_map.begin(), condition_task_id_map.end(),
        back_inserter(task_ids),
        [](auto & it) { return it.second; });
      Executor::get_instance().spin_until_task_complete(task_ids, timeout);
    }

  private:
    std::map<uint64_t, size_t> condition_task_id_map;

  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__WAITSET_HPP_
