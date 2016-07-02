#ifndef CMBML__UTILITY__EXECUTOR_HPP_
#define CMBML__UTILITY__EXECUTOR_HPP_

#include <chrono>
#include <functional>
#include <future>

#include <cmbml/types.hpp>

namespace cmbml {

struct TimedTask {
  std::function<void()> callback;
  std::chrono::nanoseconds period;
  std::chrono::steady_clock::time_point last_called_time;
  bool oneshot = false;
};

// Single-threaded synchronous round-robin executor using std::thread library (POSIX)
class SyncExecutor {

public:
  // Maybe propagate return codes out of here
  template<typename CallbackT, typename ...Args>
  void add_task(CallbackT && callback, Args &&... args) {
    // Wrap it in a bound lambda that takes no arguments
    task_list.push_back(
      [callback, &args...]() {
        callback(args...);
      }
    );
  }

  template<typename CallbackT, typename ...Args>
  void add_timed_task(
      const std::chrono::nanoseconds & timeout, bool oneshot, CallbackT && callback, Args &&... args)
  {
    timed_tasks.push_back(
      TimedTask{
        [callback, &args...]() {
          callback(args...);
        },
        timeout,
        std::chrono::steady_clock::now(),
        oneshot
      }
    );
    // Keep track of the shortest timeout period.
    min_timeout = std::min(min_timeout, timeout);
  }

  void spin() {
    // May want to reset the "start time" of all timed tasks here.
    while (true) {
      for (auto & task : timed_tasks) {
        // Check if it's time to execute.
        if (std::chrono::steady_clock::now() - task.last_called_time  >= task.period) {
          task.callback();
          task.last_called_time = std::chrono::steady_clock::now();
        }
      }

      auto timeslice = min_timeout / task_list.size();

      // TODO: I think we could get some savings if we don't make a new future every time
      // In fact this might be logically incorrect
      for (auto & task : task_list) {
        auto task_future = std::async(std::launch::async, task);
        auto future_status = task_future.wait_for(timeslice);
        if (future_status == std::future_status::ready) {
          task_future.get();
        }
      }
    }
  }

private:
  List<TimedTask> timed_tasks;
  List<std::function<void()>> task_list;
  // TODO Infer a good timeout value from the minimum wakeup period of tasks
  //
  std::chrono::nanoseconds min_timeout;
};

// Random:
// We'll need to implement a "waitset" for rmw
// please address multithreading and protecting shared memory

}

#endif  // CMBML__UTILITY__EXECUTOR_HPP_
