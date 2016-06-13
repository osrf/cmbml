#ifndef CMBML__HEADER__HPP_
#define CMBML__HEADER__HPP_

#include <cmbml/types.hpp>
#include <boost/hana/define_struct.hpp>

namespace cmbml {

struct Header {
  BOOST_HANA_DEFINE_STRUCT(Header,
    (ProtocolId_t, protocol),
    (ProtocolVersion_t, version),
    (VendorId_t, vendor_id),
    (GuidPrefix_t, guid_prefix));
};

}


#endif  // CMBML__HEADER__HPP_
