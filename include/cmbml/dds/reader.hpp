#ifndef CMBML__DDS__READER_HPP_
#define CMBML__DDS__READER_HPP_

#include <cmbml/behavior/reader_state_machine.hpp>
#include <cmbml/dds/endpoint.hpp>
#include <cmbml/dds/sample_info.hpp>
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

  // Combines serialize/deserialize, state machine, etc.
  // DataReader and DataWriter take an Executor, which abstracts the threading model.
  template<typename TopicT, typename OptionsMap, OptionsMap & options_map>
  class DataReader : public EndpointBase {
    using ReaderT = RTPSReader<OptionsMap, options_map>;
  public:
    DataReader(Participant & p) : rtps_reader(p), EndpointBase(rtps_reader.guid) {

    }

    // Simplification:
    // sample states, view states, instance states are all ANY
    // So we just pop in order, minimum sequence number first!
    StatusCode take(
        List<TopicT> & data_values, List<SampleInfo> & sample_infos, uint32_t max_samples)
    {
      if (rtps_reader.reader_cache.empty()) {
        return StatusCode::no_data;
      }
      size_t sample_length = std::min(max_samples, rtps_reader.reader_cache.size_of_cache());
      data_values.reserve(sample_length);
      for (size_t i = 0; i < sample_length; ++i) {
      // for (auto & data_value : data_values) {
        CacheChange change = rtps_reader.reader_cache.remove_change(
            rtps_reader.reader_cache.get_min_sequence_number());
        if (change.serialized_data.size() == 0) {
          // Should we not increment i in this case?
          continue;
        }
        // Convert the CacheChange to a TopicT
        if (deserialize(data_values[i], change.serialized_data) != StatusCode::ok) {
          return StatusCode::deserialize_failed;
        }
      }
      // TODO SampleInfos
      return StatusCode::ok;
    }

    // Hmm
    List<CacheChange> on_take() {
      List<CacheChange> ret = rtps_reader.reader_cache.get_filtered_cache_changes(
        &DataReader::dds_filter);
      rtps_reader.reader_cache.clear();  // This seems a little extreme!
      return ret;
    }

    // This is an initialization step. It can only be called once--enforce this.
    template<typename Context, typename Executor>
    void add_tasks(Context & thread_context, Executor & executor) {
      // TODO Initialize receiver locators
      auto receiver_thread = [this, &thread_context]() {
        // This is a blocking call
        thread_context.receive_packet(
            [&](const auto & packet) { deserialize_message(packet, thread_context); }
        );
      };
      executor.add_task(receiver_thread);

      auto heartbeat_response_delay_event = [this, &thread_context]() {
        // TODO need to reference declaration in state machine?
        boost::msm::lite::state<class must_ack> must_ack_s;
        if (state_machine.is(must_ack_s)) {
          reader_events::heartbeat_response_delay<ReaderT, Context>
            e{rtps_reader, thread_context};
          state_machine.process_event(e);
        }
      };
      executor.add_timed_task(
        rtps_reader.heartbeat_response_delay.to_ns(), false, heartbeat_response_delay_event);
    }
  protected:
    // DataReader(const DataReader &) = delete;

    // TODO duplicated in writer
    template<typename SrcT, typename NetworkContext = udp::Context>
    void deserialize_message(const SrcT & src, NetworkContext & context) {
      size_t index = 0;
      Header header;
      StatusCode deserialize_status = deserialize(header, src, index);
      if (header.protocol != rtps_protocol_id) {
        // return StatusCode::packet_invalid;
        return;
      }
      MessageReceiver receiver(header.guid_prefix, NetworkContext::kind, context.address_as_array());
      while (index <= src.size() && deserialize_status == StatusCode::ok) {
        deserialize_status = deserialize_submessage(src, index, receiver);

        conditionally_execute<ReaderT::reliability_level == ReliabilityKind_t::reliable>::call(
            process_guard_conditions, rtps_reader, state_machine);
      }
    }


    template<typename SrcT>
    StatusCode deserialize_submessage(
      const SrcT & src, size_t & index, MessageReceiver & receiver)
    {
      // TODO Skip to next submessage based on header.submessage_length
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
      return hana::eval_if(ReaderT::reliability_level == ReliabilityKind_t::reliable,
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
      return hana::eval_if(ReaderT::reliability_level == ReliabilityKind_t::reliable,
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
      cmbml::reader_events::data_received<ReaderT> e{rtps_reader, std::move(data), receiver};
      state_machine.process_event(e);
      return StatusCode::ok;
    }


    // TODO should be a lambda that the user passes in. Does it act directly on CacheChange?
    bool dds_filter(const CacheChange & change) {
      return true;
    }

    ReaderT rtps_reader;
    boost::msm::lite::sm<typename ReaderT::StateMachineT> state_machine;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__READER_HPP_
