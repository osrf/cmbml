#ifndef CMBML__DDS__WRITER_HPP_
#define CMBML__DDS__WRITER_HPP_

#include <cmbml/behavior/writer_state_machine.hpp>
#include <cmbml/serialization/serialize_cdr.hpp>
#include <cmbml/serialization/deserialize_cdr.hpp>
#include <cmbml/dds/endpoint.hpp>
#include <cmbml/discovery/endpoint/messages.hpp>
#include <cmbml/structure/writer.hpp>
#include <cmbml/structure/writer_proxy.hpp>

#include <cmbml/psm/udp/transport.hpp>

#include <cmbml/utility/executor.hpp>

namespace cmbml {
namespace dds {

  class WriterBase {
  public:
    WriterBase(const String & topic) : topic_name(topic)
    {
    }

    virtual void match_proxy(ReaderProxyPOD && reader) = 0;

    virtual const GUID_t & get_guid() const = 0;

    const String & get_topic_name() const {
      return topic_name;
    }
  protected:
    String topic_name;
  };

  // Combines serialize/deserialize, state machine, etc.
  // template<typename TopicT, typename RTPSWriter>
  template<typename TopicT, typename WriterOptions>
  class DataWriter : public WriterBase {
    using WriterT = RTPSWriter<WriterOptions>;
  public:

    // Consider taking a TransportT for this thread in the constructor.
    DataWriter(const String & topic_name, Participant & p) :
      WriterBase(topic_name), rtps_writer(p) {
    }

    InstanceHandle_t register_instance(TopicT & data) {
      // TODO we just plain ignoring InstanceHandle right now
      // I think that could make the discovery writer a lot more efficient though
      return instance_handle;
    }

    template<typename TransportT>
    StatusCode write(TopicT && data, TransportT & transport) {
      SerializedData packet;
      packet.resize(get_packet_size(data));
      CMBML__DEBUG("Writing packet of size %u\n", packet.size());
      serialize(data, packet);
      // Need to wrap data in a message type in can_send event
      InstanceHandle_t tmp{{0}};
      rtps_writer.add_change(ChangeKind_t::alive, std::move(packet), tmp);

      // Consider doing this in a different thread as indicated by sequence chart in spec?
      can_send<WriterT, TransportT> e{rtps_writer, transport};
      state_machine.process_event(e);
      return StatusCode::ok;
    }

    template<typename TransportT, typename Executor>
    void add_tasks(TransportT & transport, Executor & executor) {
      // TODO thread safety in transport!
      rtps_writer.unicast_locator_list.emplace_back(Locator_t{
        LocatorKind::udpv4,
        static_cast<uint32_t>(udp::default_user_unicast_port(cmbml_default_domain_id,
          rtps_writer.participant.participant_port_id)),
        transport.address_as_array()
      });

      rtps_writer.multicast_locator_list.emplace_back(Locator_t{
        LocatorKind::udpv4,
        static_cast<uint32_t>(udp::default_user_multicast_port(cmbml_default_domain_id)),
        transport.address_as_array()
      });

      // Configure unicast and multicast locators
      /*
      rtps_writer.for_each_matched_unicast_locator(
        [&transport](auto & locator) {
          transport.add_unicast_receiver(locator);
        }
      );
      rtps_writer.for_each_matched_multicast_locator(
        [&transport](auto & locator) {
          transport.add_multicast_receiver(locator);
        }
      );
      */


      auto receiver_thread = [this, &transport](const auto & timeout) {
        // This is a blocking call
        CMBML__DEBUG("Waiting on packet...\n");
        return transport.receive_packet(
          [&](const auto & packet) {
            CMBML__DEBUG("message came in! Deserializing...\n");
            deserialize_message(packet, transport);
          },
          timeout
        );
      };
      executor.add_task(receiver_thread);

      auto nack_response_delay_event = [this]() {
        boost::msm::lite::state<class must_repair> must_repair_s;
        if (state_machine.is(must_repair_s)) {
          CMBML__DEBUG("Nack response delay event triggered...\n");
          after_nack_delay e;
          state_machine.process_event(e);
        }
      };
      executor.add_timed_task(
        rtps_writer.nack_response_delay.to_ns(), false, nack_response_delay_event);

      hana::eval_if(WriterT::reliability == ReliabilityKind_t::reliable,
        [this, &executor, &transport]() {
          executor.add_timed_task(
            rtps_writer.heartbeat_period.to_ns(), false,
            [this, &transport]() {
              cmbml::after_heartbeat<WriterT, TransportT> e{rtps_writer, transport};
              state_machine.process_event(e);
            }
          );
        },
        [](){}
      );
    }

    void match_proxy(GUID_t && guid, SpdpDiscoData & data) {
      match_proxy({guid, data.expects_inline_qos, data.metatraffic_unicast_locator_list,
          data.metatraffic_multicast_locator_list});
    }
    void match_proxy(ReaderProxyPOD && reader) {
      configured_reader<WriterT> e{rtps_writer, std::move(reader)};
      state_machine.process_event(e);
    }
    const GUID_t & get_guid() const {
      return rtps_writer.guid;
    }

    DiscoWriterData convert_to_writer_data() const {
      // TODO other fields in this struct are not used right now.
      // This will prevent compatibility with other impls
      PublicationBuiltinTopicData publication_data;
      publication_data.topic_name = get_topic_name();
      WriterProxyPOD proxy = {
        get_guid(), rtps_writer.unicast_locator_list, rtps_writer.multicast_locator_list};
      return {publication_data, proxy};
    }

  protected:
    // TODO Refine MessageReceiver logic
    template<typename SrcT, typename TransportT = udp::Transport>
    void deserialize_message(const SrcT & src, TransportT & transport) {
      size_t index = 0;
      Header header;
      StatusCode deserialize_status = deserialize(header, src, index);
      if (header.protocol != rtps_protocol_id) {
        // return StatusCode::packet_invalid;
        return;
      }
      MessageReceiver receiver(
        header.guid_prefix, TransportT::kind, transport.address_as_array());
      while (index <= src.size() && deserialize_status == StatusCode::ok) {
        deserialize_status = deserialize_submessage(src, index, receiver);
        // This is pretty awful. Need an overhaul of the structure of Writer, maybe Writer provides
        // all relevant guard conditions
        rtps_writer.for_each_matched_reader(
          [this, &transport](auto & reader) {
            if (reader.unsent_changes_not_empty) {
              unsent_changes e;
              state_machine.process_event(e);
              reader.unsent_changes_not_empty = false;
            } else if (reader.unsent_changes_empty) {
              unsent_changes_empty e;
              state_machine.process_event(e);
              reader.unsent_changes_empty = false;
            }
            if (reader.can_send) {
              can_send<WriterT, TransportT> e{rtps_writer, transport};
              state_machine.process_event(e);
              reader.can_send = false;
            }
            conditionally_execute<WriterT::reliability == ReliabilityKind_t::reliable>::
              call([this, &reader]() {
                if (reader.requested_changes_not_empty) {
                  requested_changes e;
                  state_machine.process_event(e);
                  reader.can_send = false;
                } else if (reader.requested_changes_empty) {
                  requested_changes_empty e;
                  state_machine.process_event(e);
                  reader.requested_changes_empty = false;
                }
              }
            );

            conditionally_execute<WriterT::reliability == ReliabilityKind_t::reliable
              && WriterT::stateful>::call(
              [this](auto & reader) {
                if (reader.unacked_changes_not_empty) {
                  unacked_changes e;
                  state_machine.process_event(e);
                  reader.unacked_changes_not_empty = false;
                } else if (reader.unacked_changes_empty) {
                  unacked_changes_empty e;
                  state_machine.process_event(e);
                  reader.unacked_changes_empty = false;
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
      cmbml::acknack_received<WriterT> e{rtps_writer, std::move(acknack), receiver};
      state_machine.process_event(std::move(e));
      return StatusCode::ok;
    }

    StatusCode on_info_source(InfoSource && info_src, MessageReceiver & receiver) {
      receiver.source_guid_prefix = info_src.guid_prefix;
      receiver.source_version = info_src.protocol_version;
      receiver.source_vendor_id = info_src.vendor_id;
      // receiver.unicast_reply_locator_list = {{0}};
      // receiver.multicast_reply_locator_list = {{0}};
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
      if (WriterT::topic_kind == TopicKind_t::no_key) {
        return;
      }
      rtps_writer.add_change(ChangeKind_t::not_alive_disposed, instance_handle);
    }

    void on_unregister() {
      if (WriterT::topic_kind == TopicKind_t::no_key) {
        return;
      }
      rtps_writer.add_change(ChangeKind_t::not_alive_unregistered, instance_handle);
    }


    WriterT rtps_writer;
    InstanceHandle_t instance_handle;
    boost::msm::lite::sm<typename WriterT::StateMachineT> state_machine;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__WRITER_HPP_
