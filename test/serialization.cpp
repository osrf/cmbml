#include <iostream>

#include <array>
#include <cassert>

#include <boost/hana/for_each.hpp>

#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/cdr/deserialize_anything.hpp>
#include <cmbml/cdr/deserialize_submessage.hpp>

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
  // cmbml::convert_representations<uint32_t>(example);
  cmbml::convert_representations(example_src, example_dst, example_dst.begin());

  uint32_t dst = 0;
  uint8_t src = 1;
  size_t index = 0;
  // cmbml::place_integral_type(src, dst, index);

  // Serialization test
  // TODO compile-time inference of the serialized array for fixed sizes
  //
  std::array<uint32_t, 512> serialized_data;

  cmbml::serialize(3, serialized_data);

  cmbml::serialize(example_src, serialized_data);

  cmbml::SubmessageHeader sub_header;
  cmbml::serialize(sub_header, serialized_data);

  /*
  cmbml::Message<cmbml::AckNack, cmbml::Data, cmbml::DataFrag, cmbml::Gap,
    cmbml::Heartbeat, cmbml::HeartbeatFrag, cmbml::InfoDestination,
    cmbml::InfoReply, cmbml::InfoSource, cmbml::InfoTimestamp, cmbml::NackFrag> message;
  */
  cmbml::Message message;
  hana::tuple<cmbml::AckNack, cmbml::Data, cmbml::DataFrag, cmbml::Gap,
    cmbml::Heartbeat, cmbml::HeartbeatFrag, cmbml::InfoDestination,
    cmbml::InfoReply, cmbml::InfoSource, cmbml::InfoTimestamp, cmbml::NackFrag> types;

  hana::for_each(types, [&message](const auto x) {
      // Instantiate something with the specified type.
      cmbml::Submessage s;
      s.element = x;
      message.messages.push_back(s);
    });


  cmbml::serialize(message, serialized_data);

  // Packet comes in: we can't deduce the return type from its value
  
  cmbml::deserialize(serialized_data, message);
}
