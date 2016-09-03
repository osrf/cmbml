#ifndef CMBML__UTILITY__EXECUTOR_HPP_
#define CMBML__UTILITY__EXECUTOR_HPP_

#include <chrono>
#include <functional>
#include <future>

#include <cmbml/dds/condition.hpp>
#include <cmbml/types.hpp>

namespace cmbml {

  struct Task {
    Task(std::function<StatusCode(const std::chrono::nanoseconds&)> && callback_, bool oneshot_) :
        callback(callback_), oneshot(oneshot_)
    {
    }

    std::function<StatusCode(const std::chrono::nanoseconds &)> callback;
    bool oneshot = false;
  };

  struct FuturesTask : public Task {
    
  };

  struct TimedTask {
    TimedTask(
      std::function<void()> && callback,
      bool oneshot,
      const std::chrono::nanoseconds & period_,
      const std::chrono::steady_clock::time_point & start_time) :
        callback(std::forward<std::function<void()>>(callback)),
        oneshot(oneshot),
        period(period_), last_called_time(start_time)
    {
    }

    std::function<void()> callback;
    bool oneshot = false;
    // Trigger when the timer is first created.
    bool trigger_initially = false;
    std::chrono::nanoseconds period;
    std::chrono::steady_clock::time_point last_called_time;
  };

  // Executor concept:
  // Has functions: add_task, add_timed_task, spin_until_task_complete, remove_task

  // A multithreaded executor with futures will be easier to debug
  class FuturesExecutor {
  public:
    // Return the task_id (index into the vector)
    template<typename CallbackT, typename ...Args>
    size_t add_task(bool oneshot, CallbackT && callback) {
    }

    template<typename CallbackT, typename ...Args>
    void add_timed_task(
      const std::chrono::nanoseconds & timeout, bool oneshot,
      CallbackT && callback, Args &&... args,
      bool trigger_on_creation = false)
    {
    }

    StatusCode remove_task(size_t index) {
    }

    StatusCode spin_until_task_complete(
      List<size_t> & task_ids, const std::chrono::nanoseconds & timeout)
    {
    }


  private:
    // Have some futures
    List<TimedTask> timed_tasks;
    List<Task> task_list;
    // TODO Infer a good timeout value from the minimum wakeup period of tasks
    std::atomic<bool> running;
  };

}

#endif  // CMBML__UTILITY__EXECUTOR_HPP_
