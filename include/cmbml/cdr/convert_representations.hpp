#ifndef CMBML__CONVERT_REPRESENTATIONS__HPP_
#define CMBML__CONVERT_REPRESENTATIONS__HPP_

#include <type_traits>

#include <boost/hana/map.hpp>

#include <cmbml/cdr/place_integral_type.hpp>

namespace hana = boost::hana;

namespace cmbml {

// I couldn't find a compile-time association like this in the standard library.
constexpr auto max_value_map = hana::make_map(
    hana::make_pair(hana::type_c<uint8_t>, UINT8_MAX),
    hana::make_pair(hana::type_c<uint16_t>, UINT16_MAX),
    hana::make_pair(hana::type_c<uint32_t>, UINT32_MAX)
);

// TODO Make destination type generic for dynamically sized arrays...

// If the representation of the source type is smaller than the destination type
// TODO Unit test me
template<typename DstT, typename SrcT, size_t SrcLength, size_t DstLength,
  typename std::enable_if<sizeof(SrcT) <= sizeof(DstT) - 1>::type * = nullptr>
void convert_representations(
    const std::array<SrcT, SrcLength> & src,
    std::array<DstT, DstLength> & dst,
    typename std::array<DstT, DstLength>::iterator begin_iterator)
{
  // How many SrcT's can we fit into a DstT?
  const size_t pack_count = number_of_bits<DstT>() % number_of_bits<SrcT>();

  // Determine how many entries we are going to fill
  constexpr size_t converted_elements = sizeof(SrcT)*SrcLength/sizeof(DstT);
  static_assert(
      converted_elements <= DstLength,
      "Tried to stuff too many elements into destination representation");

  size_t src_index = 0;

  std::for_each(begin_iterator, begin_iterator + converted_elements,
    [src, &src_index, pack_count](auto & entry) {
      // Place the lower indices into the most significant bit.
      for (size_t i = 0; i < pack_count; ++i) {
        entry |= src[src_index + i] << (pack_count - 1 - i)*number_of_bits<SrcT>();
      }
      src_index += pack_count;
    }
  );
}

// Specialization if the representation of the destination type is smaller than the source type
// TODO Unit test me
template<typename DstT, typename SrcT, size_t SrcLength, size_t DstLength,
  typename std::enable_if<sizeof(DstT) <= sizeof(SrcT) - 1>::type * = nullptr>
void convert_representations(
    const std::array<SrcT, SrcLength> & src,
    std::array<DstT, DstLength> & dst,
    typename std::array<DstT, DstLength>::iterator dst_iterator)
{
  // How many DstT's are split from a SrcT?
  const size_t unpack_count = number_of_bits<SrcT>() % number_of_bits<DstT>();
  constexpr size_t converted_elements = sizeof(SrcT)*SrcLength/sizeof(DstT);
  static_assert(
      converted_elements <= DstLength,
      "Tried to stuff too many elements into destination representation");

  // size_t dst_index = 0;

  auto src_iterator = src.begin();
  std::for_each(src_iterator, src_iterator + converted_elements,
        [unpack_count, &dst, &dst_iterator](auto & entry) {
          for (size_t i = 0; i < unpack_count; ++i) {
            // MSB of entry goes into the lower index of dst
            // TODO Get max SrcT
            SrcT mask = max_value_map[hana::type_c<SrcT>] >> (number_of_bits<SrcT>() - number_of_bits<DstT>());
            SrcT shifted_entry = entry >> (unpack_count - 1 - i)*number_of_bits<DstT>();
            //dst[dst_index + i] = shifted_entry & mask;

            dst[std::distance(dst.begin(), dst_iterator)] = shifted_entry & mask;
            ++dst_iterator;
          }
          // dst_iterator += unpack_count;
        }
      );
}

// Specialiation if dst and src have the same type and size
template<typename DstT, typename SrcT, size_t SrcLength, size_t DstLength,
  typename std::enable_if<sizeof(DstT) == sizeof(SrcT)>::type * = nullptr>
void convert_representations(
    const std::array<SrcT, SrcLength> & src,
    std::array<DstT, DstLength> & dst,
    typename std::array<DstT, DstLength>::iterator & begin_iterator)
{
  assert(begin_iterator == dst.begin());
  dst = src;
}

}

#endif  // CMBML__CONVERT_REPRESENTATIONS__HPP_
