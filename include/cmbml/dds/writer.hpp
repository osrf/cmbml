#ifndef CMBML__DDS__WRITER_HPP_
#define CMBML__DDS__WRITER_HPP_

#include <cmbml/behavior/writer_state_machine.hpp>
#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/cdr/deserialize_anything.hpp>
#include <cmbml/structure/writer.hpp>

#include <cmbml/psm/udp/context.hpp>

#include <cmbml/utility/executor.hpp>

namespace cmbml {
namespace dds {

  // Combines serialize/deserialize, state machine, etc.
  template<typename RTPSWriter, typename Context = udp::Context, typename Executor = SyncExecutor>
  class DataWriter {
  public:
    // Consider taking a Context for this thread in the constructor.
    DataWriter() {
      // TODO: Dependency-inject PSM
    }

    void add_tasks(Executor & executor) {
      // TODO We really only want to have one context per thread.
      // Currently this is an overestimation.
      // TODO thread safety!
      Context thread_context;
      // TODO Initialize receiver locators
      auto receiver_thread = [this, &thread_context]() {
        // This is a blocking call
        thread_context.receive_packet(
            [&](const auto & packet) { deserialize_message(packet, thread_context); }
        );
      };
      executor.add_task(receiver_thread);

      hana::eval_if(RTPSWriter::reliability_level == ReliabilityKind_t::reliable,
        [this, &executor, &thread_context]() {
          executor.add_timed_task(
            rtps_writer.heartbeat_period.to_ns(), false,
            [this, &thread_context]() {
              cmbml::after_heartbeat<RTPSWriter, Context> e{rtps_writer, thread_context};
              state_machine.process_event(e);
            }
          );
        },
        [](){}
      );
    }

    // TODO hooks for user callback, etc.
    void on_write(Data && data) {
      // TODO state machine events?
      rtps_writer.add_change(ChangeKind_t::alive, data, instance_handle);
    }

    void on_dispose() {
      if (RTPSWriter::topic_kind == TopicKind_t::no_key) {
        return;
      }
      rtps_writer.add_change(ChangeKind_t::not_alive_disposed, instance_handle);
    }

    void on_unregister() {
      if (RTPSWriter::topic_kind == TopicKind_t::no_key) {
        return;
      }
      rtps_writer.add_change(ChangeKind_t::not_alive_unregistered, instance_handle);
    }

    // TODO Refine MessageReceiver logic
    template<typename SrcT, typename NetworkContext = udp::Context>
    void deserialize_message(const SrcT & src, NetworkContext & context) {
      size_t index = 0;
      Header header;
      StatusCode deserialize_status = deserialize(header, src, index);
      if (header.protocol != rtps_protocol_id) {
        // return StatusCode::packet_invalid;
        return;
      }
      MessageReceiver receiver(header.guid_prefix, Context::kind, context.address_as_array());
      // TODO This is why we need to propagate an error code from deserialize!
      while (index <= src.size() && deserialize_status == StatusCode::ok) {
        deserialize_status = deserialize_submessage(src, index, receiver);
      }
    }

    template<typename SrcT>
    StatusCode deserialize_submessage(
        const SrcT & src, size_t & index, MessageReceiver & receiver)
    {
      StatusCode ret = StatusCode::ok;
      auto header_callback = [this, &src, &index, &ret, &receiver](SubmessageHeader & header) {
        switch (header.submessage_id) {
          case SubmessageKind::acknack_id:
            return deserialize<AckNack>(src, index,
              [this, &receiver](auto && acknack){
                return on_acknack(std::move(acknack), receiver);
              }
            );
          case SubmessageKind::info_ts_id:
            return deserialize<InfoTimestamp>(src, index,
              [this, &receiver](auto && timestamp) {
                return on_info_timestamp(std::move(timestamp), receiver);
              }
            );
          case SubmessageKind::info_src_id:
            return deserialize<InfoSource>(src, index,
              [this, &receiver](auto && info_src) {
                return on_info_source(std::move(info_src), receiver);
              }
            );
          case SubmessageKind::info_reply_ip4_id:
            return deserialize<cmbml::udp::InfoReplyIp4>(src, index,
              [this, &receiver](auto && info_reply) {
                return on_info_reply_ip4(std::move(info_reply), receiver);
              }
            );
          case SubmessageKind::info_reply_id:
            return deserialize<InfoReply>(src, index,
              [this, &receiver](auto && info_reply) {
                return on_info_reply(std::move(info_reply), receiver);
              }
            );
          case SubmessageKind::nack_frag_id:
            // Not implemented: needed for fragmentation
            // assert(false);
            return StatusCode::not_yet_implemented;
          default:
            // In the real implementation an unknown SubmessageId is ignored
            // Should scan until next submessage found
            // assert(false);
            return StatusCode::not_yet_implemented;
        }
      };
      StatusCode header_ret = deserialize<SubmessageHeader>(src, index, header_callback);
      if (header_ret != StatusCode::ok) {
        return header_ret;
      }
      return ret;
    }

  private:

    StatusCode on_acknack(AckNack && acknack, MessageReceiver & receiver) {
      cmbml::acknack_received<RTPSWriter> e{rtps_writer, std::move(acknack), receiver};
      state_machine.process_event(std::move(e));
      return StatusCode::ok;
    }

    StatusCode on_info_source(InfoSource && info_src, MessageReceiver & receiver) {
      receiver.source_guid_prefix = info_src.guid_prefix;
      receiver.source_version = info_src.protocol_version;
      receiver.source_vendor_id = info_src.vendor_id;
      receiver.unicast_reply_locator_list = {{0}};
      receiver.multicast_reply_locator_list = {{0}};
      receiver.have_timestamp = false;
      return StatusCode::ok;
    }

    StatusCode on_info_reply(InfoReply && info_reply, MessageReceiver & receiver) {
      receiver.unicast_reply_locator_list = std::move(info_reply.unicast_locator_list);
      if (info_reply.multicast_flag) {
        receiver.multicast_reply_locator_list = std::move(info_reply.multicast_locator_list);
      } else {
        receiver.multicast_reply_locator_list.clear();
      }
      return StatusCode::ok;
    }

    StatusCode on_info_reply_ip4(
        cmbml::udp::InfoReplyIp4 && info_reply, MessageReceiver & receiver)
    {
      receiver.unicast_reply_locator_list = {std::move(info_reply.unicast_locator)};
      if (info_reply.multicast_flag) {
        receiver.multicast_reply_locator_list = {std::move(info_reply.multicast_locator)};
      } else {
        receiver.multicast_reply_locator_list.clear();
      }
      return StatusCode::ok;
    }


    StatusCode on_info_timestamp(InfoTimestamp && info_ts, MessageReceiver & receiver) {
      if (!info_ts.invalidate_flag) {
        receiver.have_timestamp = true;
        receiver.timestamp = info_ts.timestamp;
      } else {
        receiver.have_timestamp = false;
      }
      return StatusCode::ok;
    }

    RTPSWriter rtps_writer;
    InstanceHandle_t instance_handle;
    boost::msm::lite::sm<typename RTPSWriter::StateMachineT> state_machine;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__WRITER_HPP_
