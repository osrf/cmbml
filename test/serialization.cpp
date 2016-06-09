#include <array>

#include <boost/hana/for_each.hpp>

#include <cmbml/cdr/serialize_anything.hpp>

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

  std::array<uint8_t, 4> example;
  // incredibly, super basic test
  auto converted_array = cmbml::convert_representations<uint32_t>(example);

  uint32_t dst = 0;
  uint8_t src = 1;
  cmbml::place_integral_type(src, dst, 2);
}
