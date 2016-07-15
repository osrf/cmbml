#include <iostream>

#include <array>
#include <cassert>

#include <boost/hana.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/type.hpp>

#include <cmbml/serialization/serialize_cdr.hpp>
#include <cmbml/serialization/deserialize_cdr.hpp>

#include <cmbml/message/data.hpp>
#include <cmbml/message/submessage.hpp>
#include <cmbml/message/message.hpp>

namespace hana = boost::hana;

int main(int argc, char** argv) {


  // Nominal place_integral_type test (widening destination)
  {
    uint32_t dst = 0;
    uint8_t src = 1;
    size_t index = 0;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
    index += 8;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == (1 << 8) + 1);
    index += 8;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == (1 << 16) + (1 << 8) + 1);
    index += 8;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == (1 << 24) + (1 << 16) + (1 << 8) + 1);
  }

  // Nominal place_integral_type test (shrinking destination)
  {
    uint32_t src = 0x01010101;
    uint8_t dst = 0;
    size_t index = 0;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
    index += 8;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
    index += 8;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
    index += 8;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
  }


  // Serialization test
  // TODO compile-time inference of the serialized array for fixed sizes
  //
  std::array<uint32_t, 1024> serialized_data;
  {

    uint16_t test_int = 3;
    cmbml::serialize(test_int, serialized_data);
    assert(serialized_data[0] == 3);


    // How to know when to call this callback?
    auto confirm_int_callback = [test_int](auto deserialized_value) {
      assert(deserialized_value == test_int);
      return cmbml::StatusCode::ok;
    };
    size_t index = 0;
    cmbml::deserialize<uint32_t>(serialized_data, index, confirm_int_callback);
  }

  {
    std::array<uint8_t, 4> example_src;
    std::array<uint32_t, 2> example_dst;

    for (uint8_t i = 0; i < example_src.size(); ++i) {
      example_src[i] = i;
    }
    auto confirm_array_callback = [example_src](auto && deserialized_array) {
      for (size_t i = 0; i < example_src.size(); ++i) {
        assert(example_src[i] == deserialized_array[i]);
      }
      return cmbml::StatusCode::ok;
    };
    cmbml::serialize(example_src, example_dst);
    assert(example_dst[0] == example_src.size());
    assert(example_dst[1] == 0x3020100);
    size_t index = 0;
    cmbml::deserialize<decltype(example_src)>(example_dst, index, confirm_array_callback);
  }

  {
    // Validate serialize/deserialize via the callback passed the deserialize
    cmbml::SubmessageHeader sub_header;
    cmbml::serialize(sub_header, serialized_data);
    // TODO validate values
    auto callback = [&sub_header](auto && x) {
      return cmbml::StatusCode::ok;
     };

    size_t index = 0;
    cmbml::deserialize<cmbml::SubmessageHeader>(serialized_data, index, callback);
  }


  {
    size_t index = 0;
    cmbml::Submessage<cmbml::AckNack> submsg;
    cmbml::serialize(submsg, serialized_data);
    auto callback = [](auto && msg) {
      return cmbml::StatusCode::ok;
    };
    cmbml::deserialize<cmbml::AckNack>(serialized_data, index, callback);
  }

  hana::tuple<cmbml::AckNack, cmbml::Data, cmbml::Gap,
    cmbml::Heartbeat, cmbml::InfoDestination,
    cmbml::InfoReply, cmbml::InfoSource, cmbml::InfoTimestamp, cmbml::NackFrag> types;

  hana::for_each(types, [&serialized_data](const auto & x) {
    cmbml::serialize(x, serialized_data);
    using MsgT = std::decay<decltype(x)>;
    size_t index = 0;
    cmbml::deserialize(x, serialized_data, index);
  });


  printf("All tests passed.\n");
  return 0;
}
