#ifndef CMBML__DDS__CONDITION_HPP_
#define CMBML__DDS__CONDITION_HPP_

namespace cmbml {
namespace dds {
  // this is a bitmask again...
  struct StatusKind {
    // TODO
  };

  class Condition {
  public:
    Condition() : trigger_value(false)
    {
    }

    /*
    Condition(const Condition & c) : trigger_value(c.trigger_value.load())
    {
    }

    Condition(Condition && c) : trigger_value(c.trigger_value.load())
    {
    }

    void operator=(const Condition & c)
    {
      trigger_value = c.trigger_value.load();
    }
    */

    bool get_trigger_value() const {
      return trigger_value.load();
    }

  protected:
    std::atomic_bool trigger_value;
  };

  class GuardCondition : public Condition {
  public:
    void set_trigger_value() {
      trigger_value.store(true);
      cv.notify_all();
    }

    bool get_and_reset_trigger_value() {
      return trigger_value.exchange(false);
    }

    // Blocking call that waits until the guard condition is triggered
    void wait_until_trigger() {
      std::unique_lock<std::mutex> lock(cv_mutex);
      cv.wait(lock, [this]() {return trigger_value.load();});
    }

    void wait_until_trigger(const std::chrono::nanoseconds & timeout) {
      std::unique_lock<std::mutex> lock(cv_mutex);
      cv.wait_for(lock, timeout, [this]() { return trigger_value.load();});
    }

  protected:
    std::mutex cv_mutex;
    std::condition_variable cv;
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
  public:
    ReadCondition(ReaderT & reader_) : reader(reader_)
    {
    }

    ReaderT & get_data_reader() {
      return reader;
    }

    // TODO don't think a full impl of StatusCondition and ReaderCondition is needed

  protected:
    ReaderT & reader;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__CONDITION_HPP_
