#ifndef CMBML__DDS__WAITSET_HPP_
#define CMBML__DDS__WAITSET_HPP_

#include <chrono>
#include <cmbml/dds/condition.hpp>

namespace cmbml {
namespace dds {


  class Waitset {
  public:
    void attach_condition() {
      // TODO
    }

    void detach_condition() {
      // TODO
    }

    List<Condition> get_conditions() {
      // TODO
    }

    void wait(std::chrono::nanosecond & timeout) {
      // TODO
      // Need to have one thread per condition variable to see which one comes in first?
      // executor would help determine this
    }

  private:

  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__WAITSET_HPP_
