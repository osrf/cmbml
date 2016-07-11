#ifndef CMBML__UTILITY__EXECUTOR_HPP_
#define CMBML__UTILITY__EXECUTOR_HPP_

#include <chrono>
#include <functional>
#include <future>

#include <cmbml/dds/condition.hpp>
#include <cmbml/types.hpp>

namespace cmbml {

  struct Task {
    Task(
      std::function<void()> && callback_,
      bool oneshot_) : callback(callback_), oneshot(oneshot_)
    {
    }

    std::function<void()> callback;
    bool oneshot = false;
  };

  struct TimedTask : Task {
    TimedTask(
      std::function<void()> && callback,
      bool oneshot,
      const std::chrono::nanoseconds & period_,
      const std::chrono::steady_clock::time_point & start_time) :
        Task{std::forward<std::function<void()>>(callback), oneshot},
        period(period_), last_called_time(start_time)
    {
    }
    std::chrono::nanoseconds period;
    std::chrono::steady_clock::time_point last_called_time;
  };

  // Executor concept:
  // Singleton
  // Has functions: add_task, add_timed_task, spin, remove_task (?)

  // Single-threaded synchronous round-robin executor using std::thread library (POSIX)
  class SyncExecutor {

  public:
    static SyncExecutor & get_instance() {
      static SyncExecutor instance;
      return instance;
    }

    static std::mutex & get_instance_mutex() {
      // A multi-threaded Executor should use this in add_task, add_timed_task, and spin to ensure
      // that the Executor's state is thread-safe
      static std::mutex instance_mutex;
      return instance_mutex;
    }

    // Maybe propagate return codes out of here
    template<typename CallbackT, typename ...Args>
    void add_task(bool oneshot, CallbackT && callback, Args &&... args) {
      // Wrap it in a bound lambda that takes no arguments
      task_list.emplace_back(
        std::function<void()>([callback, &args...]() { callback(args...); }),
        oneshot
      );
    }

    template<typename CallbackT, typename ...Args>
    void add_task(CallbackT && callback, Args &&... args) {
      add_task(false, callback, args...);
    }

    template<typename CallbackT, typename ...Args>
    void add_timed_task(
      const std::chrono::nanoseconds & timeout, bool oneshot,
      CallbackT && callback, Args &&... args)
    {
      timed_tasks.emplace_back(
          std::function<void()>([callback, &args...]() {
            callback(args...);
          }),
          oneshot,
          timeout,
          std::chrono::steady_clock::now()
      );
      // Keep track of the shortest timeout period.
      min_timeout = std::min(min_timeout, timeout);
    }

    void spin() {
      // May want to reset the "start time" of all timed tasks here.
      running.store(true);
      while (running) {
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
          auto task_future = std::async(std::launch::async, task.callback);
          auto future_status = task_future.wait_for(timeslice);
          if (future_status == std::future_status::ready) {
            task_future.get();
          }
        }
      }
    }

    // could use a signal handler
    void interrupt() {
      running.store(false);
    }

  private:
    SyncExecutor() : min_timeout(), running(false) {
    }
    SyncExecutor(const SyncExecutor &) = delete;
    SyncExecutor(SyncExecutor &&) = delete;

    List<TimedTask> timed_tasks;
    List<Task> task_list;
    // TODO Infer a good timeout value from the minimum wakeup period of tasks
    //
    std::chrono::nanoseconds min_timeout;
    std::atomic<bool> running;
  };

  // Random:
  // We'll need to implement a "waitset" for rmw
  // please address multithreading and protecting shared memory

}

#endif  // CMBML__UTILITY__EXECUTOR_HPP_
