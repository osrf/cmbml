#ifndef CMBML__READER_PROXY__HPP_
#define CMBML__READER_PROXY__HPP_

#include <cmbml/types.hpp>

namespace cmbml {

  // Serializable POD
  struct ReaderProxyPOD {
    BOOST_HANA_DEFINE_STRUCT(ReaderProxyPOD,
      (GUID_t, remote_reader_guid),
      (bool, expects_inline_qos),
      (List<Locator_t>, unicast_locator_list),
      (List<Locator_t>, multicast_locator_list)
    );

    /*
    CMBML__DEFINE_PARAMETER_ID_MAP(ReaderProxyPOD,
      // remote_reader_guid can be omitted from the parametermap? there's no obvious id
      (expects_inline_qos, ParameterId_t::expects_inline_qos),
      (unicast_locator_list, unicast_locator),
      (multicast_locator_list, multicast_locator)
    );
    */
  };

}  // namespace cmbml

#endif  // CMBML__READER_PROXY__HPP_
