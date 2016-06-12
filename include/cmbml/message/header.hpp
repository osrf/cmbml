#ifndef CMBML__HEADER__HPP_
#define CMBML__HEADER__HPP_

#include <cmbml/types.hpp>

namespace cmbml {

struct Header {
  ProtocolId_t protocol;
  ProtocolVersion_t version;
  VendorId_t vendor_id;
  GuidPrefix_t guid_prefix;
};


}


#endif  // CMBML__HEADER__HPP_
