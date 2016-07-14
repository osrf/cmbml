#ifndef CMBML__DDS__OPTION_MAP_HPP_
#define CMBML__DDS__OPTION_MAP_HPP_

#include <boost/hana/at_key.hpp>
#include <boost/hana/map.hpp>
#include <cmbml/structure/endpoint.hpp>

namespace cmbml {

  // TODO internal linkage problem
  // This should give utilities for converting option keys into the map
  template<typename ...Options>
  constexpr auto make_option_map(Options && ...options) {
    namespace hana = boost::hana;
    return hana::make_map(options...);
  }

  // TODO These names shadow template parameter names in implementations
  template<bool Stateful, ReliabilityKind_t Reliability, TopicKind_t TopicKind, bool PushMode>
  struct WriterOptions
  {
    static const bool stateful = Stateful;
    static const ReliabilityKind_t reliability = Reliability;
    static const TopicKind_t topic_kind = TopicKind;
    static const bool push_mode = PushMode;
  };

  // This macros needs to be called in the same scope as the instantiation of the options map.
  #define CMBML__MAKE_WRITER_OPTIONS(TypeName, OptionsMap) \
    using TypeName = WriterOptions< \
      hana::at_key(OptionsMap, EndpointOptions::stateful), \
      hana::at_key(OptionsMap, EndpointOptions::reliability), \
      hana::at_key(OptionsMap, EndpointOptions::topic_kind), \
      hana::at_key(OptionsMap, EndpointOptions::push_mode)>; \

  template<bool Stateful, ReliabilityKind_t Reliability, TopicKind_t TopicKind,
    bool ExpectsInlineQos, typename TransportT>
  struct ReaderOptions
  {
    using transport = TransportT;
    static const bool stateful = Stateful;
    static const ReliabilityKind_t reliability = Reliability;
    static const TopicKind_t topic_kind = TopicKind;
    static const bool expects_inline_qos = ExpectsInlineQos;
  };

  #define CMBML__MAKE_READER_OPTIONS(TypeName, OptionsMap) \
    using TypeName = ReaderOptions< \
      hana::at_key(OptionsMap, EndpointOptions::stateful), \
      hana::at_key(OptionsMap, EndpointOptions::reliability), \
      hana::at_key(OptionsMap, EndpointOptions::topic_kind), \
      hana::at_key(OptionsMap, EndpointOptions::expects_inline_qos), \
      typename decltype(+hana::at_key(OptionsMap, EndpointOptions::transport))::type>; \


}  // namespace cmbml

#endif  // CMBML__DDS__OPTION_MAP_HPP_
