#ifndef CMBML__MESSAGE_RECEIVER__HPP_
#define CMBML__MESSAGE_RECEIVER__HPP_

#include <cmbml/message/submessage.hpp>

namespace cmbml {

struct MessageReceiver {
  ProtocolVersion_t source_version = rtps_protocol_version;
  VendorId_t source_vendor_id = vendor_id_unknown;
  GuidPrefix_t source_guid_prefix = guid_prefix_unknown;
  GuidPrefix_t dest_guid_prefix;  // TODO This is set to the participant who receives the msg
  List<Locator_t> unicast_reply_locator_list;  // TODO Set to contain a single Locator_t (see pg 35)
  List<Locator_t> multicast_reply_locator_list;  // see above
  bool have_timestamp = false;
  Time_t timestamp = time_invalid;

  // TODO Functions based on the receipt of new messages
  MessageReceiver(const GuidPrefix_t & dest_prefix, int transport_kind, const IPAddress & address) :
    dest_guid_prefix(dest_prefix)
  {
    Locator_t loc{transport_kind, 0, address};
    unicast_reply_locator_list.push_back(std::move(loc));
    Locator_t multicast_loc{transport_kind, 0, {{0}}};
    multicast_reply_locator_list.push_back(std::move(multicast_loc));
  }
};

}  // namespace cmbml

#endif  // CMBML__MESSAGE_RECEIVER__HPP_
