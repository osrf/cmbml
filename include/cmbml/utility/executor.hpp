#ifndef CMBML__UTILITY__EXECUTOR_HPP_
#define CMBML__UTILITY__EXECUTOR_HPP_

#include <cmbml/types.hpp>
#include <functional>

namespace cmbml {

// Input: a tuple of callback functions

// Single-threaded synchronous executor.
class SyncExecutor {

  // Maybe propagate return codes out of here
  template<typename CallbackT, typename...Args>
  void add_task(CallbackT && callback, Args &&... args) {
    // Wrap it in a bound lambda that takes no arguments
    task_list.push_back(
      [callback, &args...]() {
        callback(args...);
      }
    );
  }

  // TODO Wrap in futures a bit so that we can add a timeout to each wait
  // Can also reflect priority
  void spin() {
    while (true) {
      for (auto & task : task_list) {
        task();
      }
    }
  }

private:
  List<std::function<void()>> task_list;
};

// Random:
// We'll need to implement a "waitset" for rmw
// please address multithreading and how shared memory will be protected

}

#endif  // CMBML__UTILITY__EXECUTOR_HPP_
