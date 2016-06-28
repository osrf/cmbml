#ifndef CMBML__DDS__WRITER_HPP_
#define CMBML__DDS__WRITER_HPP_

#include <cmbml/behavior/writer_state_machine.hpp>
#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/cdr/deserialize_anything.hpp>
#include <cmbml/structure/writer.hpp>

#include <cmbml/psm/udp/context.hpp>

namespace cmbml {
namespace dds {

  // Combines serialize/deserialize, state machine, etc.
  template<typename RTPSWriter>
  class DataWriter {
  public:
    DataWriter() {
      // TODO: PSM stuff
    }


    // TODO user-facing API

    template<typename SrcT, typename CallbackT>
    void deserialize_submessage(
      const SrcT & src, size_t & index, CallbackT && callback)
    {
      auto header_callback = [&src, &index](SubmessageHeader header) {
        switch (header.submessage_id) {
          case SubmessageKind::acknack_id:
            deserialize<AckNack>(src, index, &DataWriter::on_acknack);
            break;
          case SubmessageKind::info_ts_id:
            deserialize<InfoTimestamp>(src, index, &DataWriter::on_info_timestamp);
            break;
          case SubmessageKind::info_src_id:
            deserialize<InfoSource>(src, index, &DataWriter::on_info_source);
            break;
          // TODO IpV6
          case SubmessageKind::info_reply_ip4_id:
            deserialize<cmbml::udp::InfoReplyIp4>(src, index, &DataWriter::on_info_reply_ip4);
            break;
          case SubmessageKind::info_reply_id:
            deserialize<InfoReply>(src, index, &DataWriter::on_info_reply);
            break;
          case SubmessageKind::nack_frag_id:
            // Not implemented: needed for fragmentation
            assert(false);
            break;
          default:
            assert(false);
        }

      };
      deserialize<SubmessageHeader>(src, index, header_callback);
    }

  private:

    void on_acknack(AckNack && acknack) {
      cmbml::acknack_received<RTPSWriter> e;
      e.writer = rtps_writer;
      // TODO Where does receiver come from?
      e.receiver = receiver;
      e.acknack = std::move(acknack);
      state_machine.process_event(std::move(e));
    }

    void on_info_source(InfoSource && info_src) {
      receiver.source_guid_prefix = info_src.guid_prefix;
      receiver.source_version = info_src.protocol_version;
      receiver.source_vendor_id = info_src.vendor_id;
      receiver.unicast_reply_locator_list = {0};
      receiver.multicast_reply_locator_list = {0};
      receiver.have_timestamp = false;
    }

    void on_info_reply(InfoReply && info_reply) {
      receiver.unicast_reply_locator_list = std::move(info_reply.unicast_locator_list);
      if (info_reply.multicast_flag) {
        receiver.multicast_reply_locator_list = std::move(info_reply.multicast_locator_list);
      } else {
        receiver.multicast_reply_locator_list.clear();
      }
    }

    void on_info_reply_ip4(cmbml::udp::InfoReplyIp4 && info_reply) {
      receiver.unicast_reply_locator_list = {std::move(info_reply.unicast_locator)};
      if (info_reply.multicast_flag) {
        receiver.multicast_reply_locator_list = {std::move(info_reply.multicast_locator)};
      } else {
        receiver.multicast_reply_locator_list.clear();
      }
    }


    void on_info_timestamp(InfoTimestamp && info_ts) {
      if (!info_ts.invalidate_flag) {
        receiver.have_timestamp = true;
        receiver.timestamp = info_ts.timestamp;
      } else {
        receiver.have_timestamp = false;
      }
    }

    RTPSWriter rtps_writer;
    MessageReceiver receiver;
    boost::msm::lite::sm<typename RTPSWriter::StateMachineT> state_machine;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__WRITER_HPP_
