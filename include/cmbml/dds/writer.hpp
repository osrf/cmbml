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
  template<typename TopicT, typename RTPSWriter>
  class DataWriter {
  public:
    // Consider taking a Context for this thread in the constructor.
    DataWriter(Participant & p) : rtps_writer(p) {
      // TODO: Dependency-inject PSM via Participant
    }

    InstanceHandle_t register_instance(TopicT & data) {
      // TODO
      return instance_handle;
    }

    template<typename Context>
    StatusCode write(TopicT && data, InstanceHandle_t & handle, Context & context) {
      // How to check instance handle against existing data?
      if (handle == handle_nil) {
        // TODO
      }

      rtps_writer.add_change(ChangeKind_t::alive, data, handle);

      // Consider doing this in a different thread as indicated by sequence chart in spec?
      // Need context here, hmmm
      can_send<RTPSWriter, Context> e{rtps_writer, context};
      state_machine.process_event(e);
      return StatusCode::ok;
    }


    template<typename Context, typename Executor>
    void add_tasks(Context & thread_context, Executor & executor) {
      // TODO We really only want to have one context per thread.
      // Currently this is an overestimation.
      // TODO thread safety!
      // TODO Initialize receiver locators
      auto receiver_thread = [this, &thread_context]() {
        // This is a blocking call
        thread_context.receive_packet(
          [&](const auto & packet) { deserialize_message(packet, thread_context); }
        );
      };
      executor.add_task(receiver_thread);

      auto nack_response_delay_event = [this]() {
        boost::msm::lite::state<class must_repair> must_repair_s;
        if (state_machine.is(must_repair_s)) {
          after_nack_delay e;
          state_machine.process_event(e);
        }
      };
      executor.add_timed_task(
        rtps_writer.nack_response_delay.to_ns(), false, nack_response_delay_event);

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

  protected:
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
      MessageReceiver receiver(
        header.guid_prefix, NetworkContext::kind, context.address_as_array());
      while (index <= src.size() && deserialize_status == StatusCode::ok) {
        deserialize_status = deserialize_submessage(src, index, receiver);
        // This is pretty awful. Need an overhaul of the structure of Writer, maybe Writer provides
        // all relevant guard conditions
        rtps_writer.for_each_matched_reader(
          [this, &context](auto & reader) {
            if (reader.unsent_changes_not_empty.get_and_reset_trigger_value()) {
              unsent_changes e;
              state_machine.process_event(e);
            } else if (reader.unsent_changes_empty.get_and_reset_trigger_value()) {
              unsent_changes_empty e;
              state_machine.process_event(e);
            }
            if (reader.can_send.get_and_reset_trigger_value()) {
              can_send<RTPSWriter, NetworkContext> e{rtps_writer, context};
              state_machine.process_event(e);
            }
            conditionally_execute<RTPSWriter::reliability_level == ReliabilityKind_t::reliable>::call(
              [this, &reader]() {
                if (reader.requested_changes_not_empty.get_and_reset_trigger_value()) {
                  requested_changes e;
                  state_machine.process_event(e);
                } else if (reader.requested_changes_empty.get_and_reset_trigger_value()) {
                  requested_changes_empty e;
                  state_machine.process_event(e);
                }
              }
            );

            conditionally_execute<RTPSWriter::reliability_level == ReliabilityKind_t::reliable
              && RTPSWriter::stateful>::call(
              [this](auto & reader) {
                if (reader.unacked_changes_not_empty.get_and_reset_trigger_value()) {
                  unacked_changes e;
                  state_machine.process_event(e);
                } else if (reader.unacked_changes_empty.get_and_reset_trigger_value()) {
                  unacked_changes_empty e;
                  state_machine.process_event(e);
                }
              }, reader);
          }
        );

      }
    }

    template<typename SrcT>
    StatusCode deserialize_submessage(
        const SrcT & src, size_t & index, MessageReceiver & receiver)
    {
      auto header_callback = [this, &src, &index, &receiver](SubmessageHeader & header) {
        switch (header.submessage_id) {
          case SubmessageKind::acknack_id:
            return deserialize<AckNack>(src, index,
              [this, &receiver, &header](auto && acknack){
                acknack.assign_flags(header.flags);
                return on_acknack(std::move(acknack), receiver);
              }
            );
          case SubmessageKind::info_ts_id:
            return deserialize<InfoTimestamp>(src, index,
              [this, &receiver, &header](auto && timestamp) {
                timestamp.assign_flags(header.flags);
                return on_info_timestamp(std::move(timestamp), receiver);
              }
            );
          case SubmessageKind::info_src_id:
            return deserialize<InfoSource>(src, index,
              [this, &receiver, &header](auto && info_src) {
                info_src.assign_flags(header.flags);
                return on_info_source(std::move(info_src), receiver);
              }
            );
          case SubmessageKind::info_reply_ip4_id:
            return deserialize<cmbml::udp::InfoReplyIp4>(src, index,
              [this, &receiver, &header](auto && info_reply) {
                info_reply.assign_flags(header.flags);
                return on_info_reply_ip4(std::move(info_reply), receiver);
              }
            );
          case SubmessageKind::info_reply_id:
            return deserialize<InfoReply>(src, index,
              [this, &receiver, &header](auto && info_reply) {
                info_reply.assign_flags(header.flags);
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
      return deserialize<SubmessageHeader>(src, index, header_callback);
    }


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


    RTPSWriter rtps_writer;
    InstanceHandle_t instance_handle;
    boost::msm::lite::sm<typename RTPSWriter::StateMachineT> state_machine;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__WRITER_HPP_
