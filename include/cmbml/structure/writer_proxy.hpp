#ifndef CMBML__WRITER_PROXY__HPP_
#define CMBML__WRITER_PROXY__HPP_

#include <cmbml/types.hpp>

namespace cmbml {

  struct WriterProxyPOD {
    BOOST_HANA_DEFINE_STRUCT(WriterProxyPOD,
      (GUID_t, remote_writer_guid),
      (List<Locator_t>, unicast_locator_list),
      (List<Locator_t>, multicast_locator_list)
    );

    /*
    constexpr auto get_parameter_map() {
    }

    CMBML__DEFINE_PARAMETER_ID_MAP(WriterProxyPOD,
      // remote_reader_guid can be omitted from the parametermap? there's no obvious id
      (unicast_locator_list, unicast_locator),
      (multicast_locator_list, multicast_locator)
    )
    */
  };

}  // namespace cmbml

#endif  // CMBML__WRITER_PROXY__HPP_
