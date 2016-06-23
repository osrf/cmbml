#include <iostream>

#include <array>
#include <cassert>

#include <boost/hana.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/type.hpp>

#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/cdr/deserialize_anything.hpp>

#include <cmbml/message/data.hpp>
#include <cmbml/message/submessage.hpp>
#include <cmbml/message/message.hpp>

namespace hana = boost::hana;

int main(int argc, char** argv) {
  // I want to construct a tuple of arrays
  //
  // hana::tuple<uint8_t, uint16_t, uint32_t> numeric_types;
  // make a bunch of arrays; is there a nicer way? probably, using hana::types
  /*
  hana::tuple<std::array<uint8_t, 4>, std::array<uint16_t, 2>, std::array<uint8_t, 8>> input_arrays;

  // Having comparison operators between different array representations might be nice too!
  hana::for_each(input_arrays, [](auto & member){
        // TODO Fill some values too
        auto converted_array = convert_representation(member);
      });
  */

  std::array<uint8_t, 4> example_src;
  std::array<uint32_t, 1> example_dst;
  // incredibly, super basic test to see if it compiles
  cmbml::convert_representations(example_src, example_dst, example_dst.begin());

  // Nominal place_integral_type test (widening destination)
  {
    uint32_t dst = 0;
    uint8_t src = 1;
    size_t index = 0;
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
    cmbml::place_integral_type(src, dst, index);
    assert(dst == (1 << 8) + 1);
    cmbml::place_integral_type(src, dst, index);
    assert(dst == (1 << 16) + (1 << 8) + 1);
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
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
    cmbml::place_integral_type(src, dst, index);
    assert(dst == 1);
  }


  // Serialization test
  // TODO compile-time inference of the serialized array for fixed sizes
  //
  std::array<uint32_t, 1024> serialized_data;

  cmbml::serialize(3, serialized_data);
  serialized_data.fill(0);

  cmbml::serialize(example_src, serialized_data);
  serialized_data.fill(0);

  cmbml::SubmessageHeader sub_header;
  cmbml::serialize(sub_header, serialized_data);
  cmbml::SubmessageHeader sub_header_deserialized;
  cmbml::deserialize(serialized_data, sub_header_deserialized);
  serialized_data.fill(0);

  /*
  cmbml::Message<cmbml::AckNack, cmbml::Data, cmbml::DataFrag, cmbml::Gap,
    cmbml::Heartbeat, cmbml::HeartbeatFrag, cmbml::InfoDestination,
    cmbml::InfoReply, cmbml::InfoSource, cmbml::InfoTimestamp, cmbml::NackFrag> message;
  */
  cmbml::Message message;
  hana::tuple<cmbml::AckNack, cmbml::Data, cmbml::Gap,
    cmbml::Heartbeat, cmbml::InfoDestination,
    cmbml::InfoReply, cmbml::InfoSource, cmbml::InfoTimestamp, cmbml::NackFrag> types;

  hana::for_each(types, [&serialized_data](const auto & x) {
    cmbml::serialize(x, serialized_data);
    typename std::decay<decltype(x)>::type result;
    cmbml::deserialize(serialized_data, result);
    serialized_data.fill(0);
  });

  cmbml::Submessage submsg;
  submsg.element = std::make_unique<cmbml::AckNack>();
  submsg.header.submessage_id = cmbml::AckNack::id;
  cmbml::serialize(submsg, serialized_data);

  cmbml::deserialize(serialized_data, submsg);
  serialized_data.fill(0);

  hana::for_each(types, [&message](const auto & x) {
    cmbml::Submessage s;
    using ElementT = typename std::decay<decltype(x)>::type;
    s.element = std::make_unique<ElementT>();
    s.header.submessage_id = ElementT::id;
    message.messages.push_back(std::move(s));
  });

  cmbml::serialize(message, serialized_data);

  // Packet comes in: we can't deduce the return type from its value
  cmbml::Message deserialized_message;
  cmbml::deserialize(serialized_data, deserialized_message);
  serialized_data.fill(0);
}
