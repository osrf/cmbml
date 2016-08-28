#ifndef CMBML__DDS__READER_HPP_
#define CMBML__DDS__READER_HPP_

#include <memory>

#include <cmbml/behavior/reader_state_machine.hpp>
#include <cmbml/dds/endpoint.hpp>
#include <cmbml/dds/sample_info.hpp>
#include <cmbml/discovery/endpoint/messages.hpp>
#include <cmbml/serialization/serialization.hpp>
#include <cmbml/utility/metafunctions.hpp>

namespace cmbml {
namespace dds {

  auto process_guard_conditions = [](auto & reader, auto & state_machine) {
    reader.for_each_matched_writer(
      [&reader, &state_machine](auto & writer) {
        if (writer.missing_changes_empty) {
          reader_events::missing_changes_empty e;
          state_machine.process_event(e);
          writer.missing_changes_empty = false;
        }
        if (writer.missing_changes_not_empty) {
          reader_events::missing_changes_not_empty e;
          state_machine.process_event(e);
          writer.missing_changes_not_empty = false;
        }
      }
    );
  };

  // For template erasure.
  class ReaderBase {
  public:
    ReaderBase(const String & topic) : topic_name(topic)
    {
    }

    virtual void match_proxy(WriterProxyPOD && writer) = 0;

    virtual const GUID_t & get_guid() const = 0;

    virtual const String & get_topic_name() const {
      return topic_name;
    }
  protected:
    String topic_name;
  };

  // Combines serialize/deserialize, state machine, etc.
  // DataReader and DataWriter take an Executor, which abstracts the threading model.
  template<typename TopicT, typename ReaderOptions>
  class DataReader : public ReaderBase {
    using ReaderT = RTPSReader<ReaderOptions>;
  public:
    using Topic = TopicT;

    // TODO Disable copy constructor.
    DataReader(const String & topic_name, Participant & p) : ReaderBase(topic_name), rtps_reader(p)
    {
    }

    /*
    ~DataReader()
    {
      // hmmmmmm
      Domain & domain = Domain::get_instance();
      for (auto endpoint : domain.readers) {
      }
    }
    */

    // Single-message with no sample info
    StatusCode take(TopicT & data)
    {
      StatusCode status;
      take_helper(data, status);

      return status;
    }

    // Simplification:
    // sample states, view states, instance states are all ANY
    // So we just pop in order, minimum sequence number first!
    StatusCode take(
        List<TopicT> & data_values, List<SampleInfo> & sample_infos, uint32_t max_samples)
    {
      if (rtps_reader.reader_cache.size_of_cache() == 0) {
        return StatusCode::no_data;
      }
      size_t sample_length = std::min(max_samples, rtps_reader.reader_cache.size_of_cache());
      data_values.resize(sample_length);
      for (size_t i = 0; i < sample_length; ++i) {
        StatusCode status;
        take_helper(data_values[i], status);
        if (status != StatusCode::ok) {
          return status;
        }
      }
      // TODO SampleInfos
      return StatusCode::ok;
    }

    // Hmm--currently this is unused
    List<CacheChange> on_take() {
      List<CacheChange> ret = rtps_reader.reader_cache.get_filtered_cache_changes(
        &DataReader::dds_filter);
      rtps_reader.reader_cache.clear();  // This seems a little extreme!
      return ret;
    }

    // This is an initialization step. It can only be called once--enforce this.
    // TODO Get domain_id in initialization of DDS Entities and use it here
    template<typename TransportT, typename Executor>
    void add_tasks(TransportT & transport, Executor & executor) {
      // TODO Initialize receiver locators!!!
      rtps_reader.unicast_locator_list.emplace_back(Locator_t{
        LocatorKind::udpv4,
        static_cast<uint32_t>(udp::default_user_unicast_port(cmbml_default_domain_id,
            rtps_reader.participant.participant_port_id)),
        transport.address_as_array()
      });

      rtps_reader.multicast_locator_list.emplace_back(Locator_t{
        LocatorKind::udpv4,
        static_cast<uint32_t>(udp::default_user_multicast_port(cmbml_default_domain_id)),
        transport.address_as_array()}
      );
      for (auto & locator : rtps_reader.unicast_locator_list) {
        transport.add_unicast_receiver(locator);
      }
      for (auto & locator : rtps_reader.multicast_locator_list) {
        transport.add_multicast_receiver(locator);
      }

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

      auto heartbeat_response_delay_event = [this, &transport]() {
        // TODO do I need to reference declaration of this state in state machine?
        boost::msm::lite::state<class must_ack> must_ack_s;
        if (state_machine.is(must_ack_s)) {
          CMBML__DEBUG("Heartbeat response delay event triggered...\n");
          reader_events::heartbeat_response_delay<ReaderT, TransportT>
            e{rtps_reader, transport};
          state_machine.process_event(e);
        }
      };
      executor.add_timed_task(
        rtps_reader.heartbeat_response_delay.to_ns(), false, heartbeat_response_delay_event);
    }

    ReadCondition<DataReader> & create_read_condition() {
      if (!read_condition) {
        read_condition = std::make_unique<ReadCondition<DataReader>>(*this);
      }
      return *read_condition;
    }

    virtual void match_proxy(GUID_t && guid, SpdpDiscoData & data) {
      match_proxy({guid, data.metatraffic_unicast_locator_list,
          data.metatraffic_multicast_locator_list});
    }

    void match_proxy(WriterProxyPOD && writer) {
      reader_events::reader_created<ReaderT> e{rtps_reader, std::move(writer)};
      state_machine.process_event(e);
    }

    const GUID_t & get_guid() const {
      return rtps_reader.guid;
    }

    DiscoReaderData convert_to_reader_data() const {
      SubscriptionBuiltinTopicData subscription_data;
      ReaderProxyPOD proxy = {get_guid(), rtps_reader.expects_inline_qos,
        rtps_reader.unicast_locator_list, rtps_reader.multicast_locator_list};
      return {ContentFilterProperty_t(), subscription_data, proxy};
    }

  protected:
    // TODO duplicated in writer
    // TODO More messagereceiver rules.
    template<typename SrcT, typename TransportT = udp::Transport>
    void deserialize_message(const SrcT & src, TransportT & transport) {
      size_t index = 0;
      Header header;
      StatusCode deserialize_status = deserialize(header, src, index);
      if (header.protocol != rtps_protocol_id) {
        // return StatusCode::packet_invalid;
        return;
      }
      // hmmmmmmmmmmmmm
      MessageReceiver receiver(
          header.guid_prefix, TransportT::kind, transport.address_as_array());
      while (index <= src.size() && deserialize_status == StatusCode::ok) {
        deserialize_status = deserialize_submessage(src, index, receiver);

        conditionally_execute<ReaderT::reliability == ReliabilityKind_t::reliable>::call(
            process_guard_conditions, rtps_reader, state_machine);
      }
    }


    // TODO Skip to next submessage based on header.submessage_length
    template<typename SrcT>
    StatusCode deserialize_submessage(
      const SrcT & src, size_t & index, MessageReceiver & receiver)
    {
      auto header_callback = [this, &src, &index, &receiver](SubmessageHeader & header) {
        // Extract flags from the header
        switch (header.submessage_id) {
          case SubmessageKind::heartbeat_id:
            return deserialize<Heartbeat>(src, index,
              [this, &header](auto && heartbeat) {
                heartbeat.assign_flags(header.flags);
                return on_heartbeat(std::move(heartbeat));
              }
            );
          case SubmessageKind::gap_id:
            return deserialize<Gap>(src, index,
              [this, &header](auto && gap) {
                gap.assign_flags(header.flags);
                return on_gap(std::move(gap));
              });
          case SubmessageKind::info_dst_id:
            return deserialize<InfoDestination>(src, index,
              [this, &receiver, &header](auto && info_destination) {
                info_destination.assign_flags(header.flags);
                return on_info_destination(std::move(info_destination), receiver);
              }
            );
          case SubmessageKind::heartbeat_frag_id:
            // Fragmentation isn't implemented yet
            return StatusCode::not_yet_implemented;
          case SubmessageKind::data_id:
            return deserialize<Data>(src, index,
              [this, &receiver, &header](auto && data) {
                data.assign_flags(header.flags);
                return on_data(std::move(data), receiver);
              }
            );
          case SubmessageKind::data_frag_id:
            // Fragmentation isn't implemented yet
            return StatusCode::not_yet_implemented;
            break;
          default:
            // assert(false);
            return StatusCode::not_yet_implemented;
        }
      };
      return deserialize<SubmessageHeader>(src, index, header_callback);
    }


    StatusCode on_heartbeat(Heartbeat && heartbeat) {
      // TODO double-check that heartbeat comes from the matched destination...
      // In the implementation we should just emit a warning, e.g. in case someone is 
      // sending bogus packets
      return hana::eval_if(ReaderT::reliability == ReliabilityKind_t::reliable,
        [this, &heartbeat]() {
          cmbml::reader_events::heartbeat_received<ReaderT> e{rtps_reader, heartbeat};
          state_machine.process_event(e);
          return StatusCode::ok;
        },
        []() {
          return StatusCode::ok;
        }
      );
    }

    StatusCode on_gap(Gap && gap) {
      return hana::eval_if(ReaderT::reliability == ReliabilityKind_t::reliable,
        [this, &gap]() {
          cmbml::reader_events::gap_received<ReaderT> e{rtps_reader, gap};
          state_machine.process_event(e);
          return StatusCode::ok;
        },
        []() {
          return StatusCode::ok;
        }
      );
    }

    StatusCode on_info_destination(InfoDestination && info_dst, MessageReceiver & receiver) {
      if (info_dst.guid_prefix != guid_prefix_unknown) {
        // guid_prefix is pretty big (12 bytes)
        receiver.dest_guid_prefix = info_dst.guid_prefix;
      } else {
        // Set to participant's guid_prefix, which should be the same as our guid_prefix
        receiver.dest_guid_prefix = rtps_reader.guid.prefix;
      }
      return StatusCode::ok;
    }

    StatusCode on_data(Data && data, MessageReceiver & receiver) {
      cmbml::reader_events::data_received<ReaderT> e{
        rtps_reader, std::move(data), receiver};
      state_machine.process_event(e);
      // read_condition.set_trigger_value();
      return StatusCode::ok;
    }


    // Take and deserialize one message.
    // Return true if the message was deserialized
    bool take_helper(TopicT & data, StatusCode & status) {
      if (rtps_reader.reader_cache.size_of_cache() == 0) {
        status = StatusCode::no_data;
        return false;
      }
      CacheChange change = rtps_reader.reader_cache.remove_change(
          rtps_reader.reader_cache.get_min_sequence_number());
      // Skip deserialization in this case.
      if (change.serialized_data.size() == 0) {
        status = StatusCode::precondition_violated;
        return false;
      }
      // Convert the CacheChange to a TopicT
      if (deserialize_payload(data, change.serialized_data) != StatusCode::ok) {
        status =  StatusCode::deserialize_failed;
        return false;
      }
      status = StatusCode::ok;
      return true;
    }

    // TODO should be a lambda that the user passes in. Does it act directly on CacheChange?
    bool dds_filter(const CacheChange & change) {
      return true;
    }

    ReaderT rtps_reader;
    boost::msm::lite::sm<typename ReaderT::StateMachineT> state_machine;

    // TODO: Allocator
    // TODO Actually, we could have multiple read conditions
    std::unique_ptr<ReadCondition<DataReader>> read_condition = nullptr;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__READER_HPP_
