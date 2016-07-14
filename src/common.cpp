#include <cmbml/cdr/common.hpp>

namespace cmbml {

  const char * error_code_string(StatusCode code) {
    switch(code) {
      case StatusCode::ok:
        return "StatusCode::ok";
      case StatusCode::precondition_violated:
        return "StatusCode::precondition_violated";
      case StatusCode::postcondition_violated:
        return "StatusCode::postcondition_violated";
      case StatusCode::not_yet_implemented:
        return "StatusCode::not_yet_implemented";
      case StatusCode::packet_invalid:
        return "StatusCode::packet_invalid";
      case StatusCode::out_of_memory:
        return "StatusCode::out_of_memory";
      case StatusCode::deserialize_failed:
        return "StatusCode::deserialize_failed";
      case StatusCode::no_data:
        return "StatusCode::no_data";
      case StatusCode::timeout:
        return "StatusCode::timeout";
      default:
        return "ERROR: invalid status code reached";
    }
    return "";
  }

}
