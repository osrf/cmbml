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
      // but it shouldn't be publically accessible
      static std::mutex instance_mutex;
      return instance_mutex;
    }

    // Return the task_id (index into the vector)
    template<typename CallbackT, typename ...Args>
    size_t add_task(bool oneshot, CallbackT && callback) {
      // Wrap it in a bound lambda that takes no arguments
      task_list.emplace_back(
        std::function<StatusCode(const std::chrono::nanoseconds &)>(callback),
        oneshot
      );
      return task_list.size() - 1;
    }

    StatusCode remove_task(size_t index) {
      if (index >= task_list.size()) {
        return StatusCode::precondition_violated;
      }
      task_list.erase(task_list.begin() + index);
      return StatusCode::ok;
    }

    template<typename CallbackT, typename ...Args>
    size_t add_task(CallbackT && callback, Args &&... args) {
      return add_task(false, callback, std::forward<Args>(args)...);
    }

    // don't block in timed tasks!
    // consider renaming TimerTasks/add_timer_task
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

    // probably broken: focusing on spin_until_tasks_complete for now.
    // Since this is a synchronous executor, we serialize the tasks in one thread and 
    // wait on the completion of each one in even timeslice.
    void spin() {
      // May want to reset the "start time" of all timed tasks here.
      // TODO Check if oneshot and remove oneshot tasks that completed.
      /*
      running.store(true);
      while (running && !task_list.empty()) {
        for (auto & task : timed_tasks) {
          // Check if it's time to execute.
          if (std::chrono::steady_clock::now() - task.last_called_time  >= task.period) {
            task.callback();
            task.last_called_time = std::chrono::steady_clock::now();
            if (task.oneshot) {
              //remove
            }
          }
        }

        auto timeslice = min_timeout / task_list.size();

        // TODO: I think we could get some savings if we don't make a new future every time
        // In fact this might be logically incorrect
        //
        for (auto & task : task_list) {
          auto task_future = std::async(std::launch::deferred, task.callback);
          auto future_status = task_future.wait_for(timeslice);
          if (future_status == std::future_status::ready) {
            task_future.get();
            if (task.oneshot) {
              // remove
            }
          }
        }
      }
      running.store(false);
      */
    }

    // Does not consider timed tasks
    // TODO Implement timeout
    // Spin until ONE of the tasks in task_id is completed.
    void spin_until_task_complete(List<size_t> & task_ids, const std::chrono::nanoseconds & timeout)
    {
      // Make a future for each task_id
      running.store(true);
      while (running && !task_list.empty()) {
        for (auto & task : timed_tasks) {
          // Check if it's time to execute.
          if (std::chrono::steady_clock::now() - task.last_called_time  >= task.period) {
            task.callback();
            task.last_called_time = std::chrono::steady_clock::now();
            if (task.oneshot) {
              // TODO remove
            }
          }
        }

        // TODO Calculating this timeslice
        std::chrono::nanoseconds timeslice = min_timeout / task_list.size();

        // TODO: I think we could get some savings if we don't make a new future every time
        // In fact this might be logically incorrect
        for (size_t i = 0; i < task_list.size(); ++i) {
          auto status_code = task_list[i].callback(timeslice);
          for (auto & j : task_ids) {
            // Call the callback with a timeout
            // TODO Task needs to be able to indicate status (error, succes, timeout)
            if (i == j && status_code == StatusCode::ok) {
              running.store(false);
              return;
            }
          }
        }
      }
      running.store(false);
    }

    // could use a signal handler
    void interrupt() {
      running.store(false);
    }
    bool is_running() const {
      return running.load();
    }

  private:
    using FutureT = std::future<void>;
    SyncExecutor() : min_timeout(), running(false) {
    }
    SyncExecutor(const SyncExecutor &) = delete;
    SyncExecutor(SyncExecutor &&) = delete;

    List<TimedTask> timed_tasks;
    List<Task> task_list;
    // TODO Infer a good timeout value from the minimum wakeup period of tasks
    //
    std::chrono::nanoseconds min_timeout{5000000};
    std::atomic<bool> running;
  };

  // Random:
  // We'll need to implement a "waitset" for rmw
  // please address multithreading and protecting shared memory

}

#endif  // CMBML__UTILITY__EXECUTOR_HPP_
