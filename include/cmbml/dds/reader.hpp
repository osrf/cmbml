#ifndef CMBML__DDS__READER_HPP_
#define CMBML__DDS__READER_HPP_

#include <cmbml/behavior/reader_state_machine_events.hpp>

namespace cmbml {
namespace dds {

  // Combines serialize/deserialize, state machine, etc.
  // DataReader and DataWriter take an Executor, which abstracts the threading model.
  template<typename RTPSReader, typename Context = udp::Context, typename Executor = SyncExecutor>
  class DataReader {
  public:

    void add_tasks(Executor & executor) {
      // TODO Executor should dispense the contexts? Hmm
      Context thread_context;
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
          reader_events::heartbeat_response_delay<RTPSReader, Context>
            e{rtps_reader, thread_context};
          state_machine.process_event(e);
        }
      };
      executor.add_timed_task(
        rtps_reader.heartbeat_response_delay.to_ns(), false, heartbeat_response_delay_event);
    }

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
      MessageReceiver receiver(header.guid_prefix, Context::kind, context.address_as_array());
      while (index <= src.size() && deserialize_status == StatusCode::ok) {
        deserialize_status = deserialize_submessage(src, index, receiver);
      }
    }


    template<typename SrcT>
    StatusCode deserialize_submessage(
      const SrcT & src, size_t & index, MessageReceiver & receiver)
    {
      auto header_callback = [this, &src, &index, &receiver](SubmessageHeader & header) {
        switch (header.submessage_id) {
          case SubmessageKind::heartbeat_id:
            return deserialize<Heartbeat>(src, index,
              [this, &receiver](auto && heartbeat) {
                return on_heartbeat(std::move(heartbeat), receiver);
              }
            );
          case SubmessageKind::gap_id:
            return deserialize<Gap>(src, index,
              [this, &receiver](auto && gap) {
                return on_gap(std::move(gap), receiver);
              });
          case SubmessageKind::info_dst_id:
            return deserialize<InfoDestination>(src, index,
              [this, &receiver](auto && info_destination) {
                return on_info_destination(std::move(info_destination), receiver);
              }
            );
          case SubmessageKind::heartbeat_frag_id:
            // Fragmentation isn't implemented yet
            return StatusCode::not_yet_implemented;
          case SubmessageKind::data_id:
            return deserialize<Data>(src, index,
              [this, &receiver](auto && data) {
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

  private:

    List<CacheChange> on_read() {
      return rtps_reader.reader_cache.get_filtered_cache_changes(&DataReader::dds_filter);
    }
    List<CacheChange> on_take() {
      List<CacheChange> ret = rtps_reader.reader_cache.get_filtered_cache_changes(
        &DataReader::dds_filter);
      rtps_reader.reader_cache.clear();
      return ret;
    }

    StatusCode on_heartbeat(Heartbeat && heartbeat, MessageReceiver & receiver) {
      // TODO double-check that heartbeat comes from the matched destination...
      // In the implementation we should just emit a warning, e.g. in case someone is 
      // sending bogus packets
      return hana::eval_if(RTPSReader::reliability_level == ReliabilityKind_t::reliable,
        [this, &heartbeat, &receiver]() {
          cmbml::reader_events::heartbeat_received<RTPSReader> e{rtps_reader, heartbeat};
          state_machine.process_event(e);
          return StatusCode::ok;
        },
        []() {
          return StatusCode::ok;
        }
      );
    }

    StatusCode on_gap(Gap && gap, MessageReceiver & receiver) {
      return hana::eval_if(RTPSReader::reliability_level == ReliabilityKind_t::reliable,
        [this, &gap, &receiver]() {
          cmbml::reader_events::gap_received<RTPSReader> e{rtps_reader, gap};
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
      // user_data_callback(data);
      cmbml::reader_events::data_received<RTPSReader> e{rtps_reader, std::move(data), receiver};
      state_machine.process_event(e);
      return StatusCode::ok;
    }


    // TODO should be a lambda that the user passes in. Does it act directly on CacheChange?
    bool dds_filter(const CacheChange & change) {
      return true;
    }

    // MessageReceiver receiver;
    RTPSReader rtps_reader;
    boost::msm::lite::sm<typename RTPSReader::StateMachineT> state_machine;
  };
}
}

#endif  // CMBML__DDS__READER_HPP_
