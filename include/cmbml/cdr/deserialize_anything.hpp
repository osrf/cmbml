// A utility for deserializing anything (in CDR format)
#ifndef CMBML__DESERIALIZER__HPP_
#define CMBML__DESERIALIZER__HPP_

#include <boost/hana/for_each.hpp>

#include <cmbml/cdr/place_integral_type.hpp>

namespace hana = boost::hana;

namespace cmbml {

template<typename SrcT, typename DstT, class = typename SrcT::iterator,
  std::enable_if<std::is_integral<DstT>::value>::type * = nullptr>
void deserialize(const SrcT & src, DstT & dst, size_t & index, size_t & subindex)
{
  // "De-alignment"

  // TODO

  place_integral_type(src[index], dst, subindex);
}

template<
  typename SrcT,
  typename DstT,
  class = typename SrcT::iterator,
  typename std::enable_if<hana::Sequence<DstT>::value>::value * = nullptr>
void deserialize(const SrcT & src, Dst & dst, size_t & index, size_t & subindex)
{
  hana::for_each(dst, [&src, &index, &subindex](auto & element) {
      deserialize(src, element, index, subindex);
    }
  );
}

// Can we deduce a return type from the raw message?
// would be nice to enforce that SrcT is an iterable of uint32_t's
template<
  typename SrcT,
  typename DstT,
  class = typename SrcT::iterator,
  typename std::enable_if<hana::Foldable<DstT>::value>::value * = nullptr>
void deserialize(const SrcT & src, Dst & dst, size_t & index, size_t & subindex)
{
  hana::for_each(dst, [&src, &index, &subindex](auto & element) {
      deserialize(src, hana::second(element), index, subindex);
    }
  );
}


}

#endif  // CMBML__DESERIALIZER__HPP_
