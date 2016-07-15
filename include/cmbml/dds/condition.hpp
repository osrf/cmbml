#ifndef CMBML__DDS__CONDITION_HPP_
#define CMBML__DDS__CONDITION_HPP_

#include <atomic>
#include <mutex>
#include <condition_variable>

#include <cmbml/common.hpp>

namespace cmbml {
namespace dds {

  class Condition {
  public:
    Condition();

    bool get_trigger_value() const;

    // Blocking call that waits until the guard condition is triggered
    StatusCode wait_until_trigger();

    // Wait with timeout
    // TODO Give indication of timeout status
    StatusCode wait_until_trigger(const std::chrono::nanoseconds & timeout);
    const int condition_id;

  protected:
    std::atomic_bool trigger_value;
    std::mutex cv_mutex;
    std::condition_variable cv;
  };

  class GuardCondition : public Condition {
  public:
    void set_trigger_value() {
      Condition::trigger_value.store(true);
      Condition::cv.notify_one();
    }

    bool get_and_reset_trigger_value() {
      bool ret = Condition::trigger_value.exchange(false);
      if (ret) {
        Condition::cv.notify_one();
      }
      return ret;
    }
  };

  // this is a bitmask again...
  struct StatusKind {
    // TODO
  };
  template<typename T>
  class StatusCondition : public Condition {
  public:
    void set_enabled_statuses() {
      // TODO
    }

    void get_enabled_statuses() {
      // TODO
    }

    T & get_entity() {
      return entity;
    }

  private:
    T & entity;

  };

  template<typename ReaderT>
  class ReadCondition : public Condition {
    friend ReaderT;
  public:

    ReaderT & get_data_reader() {
      return reader;
    }
    // a ReadCondition can only be constructed by the owning ReaderT
    ReadCondition(ReaderT & reader_) : reader(reader_)
    {
    }

  protected:

    ReaderT & reader;

    void set_trigger_value() {
      Condition::trigger_value.store(true);
      Condition::cv.notify_one();
    }
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__CONDITION_HPP_
