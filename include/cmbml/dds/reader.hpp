#ifndef CMBML__DDS__READER_HPP_
#define CMBML__DDS__READER_HPP_

#include <cmbml/behavior/reader_state_machine_events.hpp>

namespace cmbml {
namespace dds {

  // Combines serialize/deserialize, state machine, etc.
  template<typename RTPSReader>
  class DataReader {
  public:

    template<typename SrcT, typename CallbackT>
    void deserialize_submessage(
      const SrcT & src, size_t & index, CallbackT && callback)
    {
      // Could be phrased more functionally if, once I figure out how callbacks chain via lambdas,
      // if this were done by traversing the Submessage type?
      auto header_callback = [&src, &index](SubmessageHeader header) {
        // TODO 
        switch (header.submessage_id) {
          case SubmessageKind::heartbeat_id:
            deserialize<Heartbeat>(src, index, &DataReader::on_heartbeat);
            break;
          case SubmessageKind::gap_id:
            deserialize<Gap>(src, index, &DataReader::on_gap);
            break;
          case SubmessageKind::info_dst_id:
            deserialize<InfoDestination>(src, index, &DataReader::on_info_destination);
            break;
          case SubmessageKind::heartbeat_frag_id:
            // Fragmentation isn't implemented yet
            assert(false);
            break;
          case SubmessageKind::data_id:
            deserialize<Data>(src, index, &DataReader::on_data);
            // CMBML__DESERIALIZE_SUBMESSAGE_BY_TYPE(Data);
            break;
          case SubmessageKind::data_frag_id:
            // Fragmentation isn't implemented yet
            assert(false);
            break;
          default:
            assert(false);
        }
      };
      deserialize<SubmessageHeader>(src, index, header_callback);
    }
  private:

    void on_heartbeat(Heartbeat && heartbeat) {
      // TODO double-check that heartbeat comes from the matched destination...
      // Maybe we actually need to craft the 
      // In the implementation we should just emit a warning, e.g. in case someone is 
      // sending bogus packets
      GUID_t writer_guid = {receiver.dest_guid_prefix, heartbeat.writer_id};
      WriterProxy * proxy = reader.matched_writer_lookup(writer_guid);
      assert(proxy);
      cmbml::reader_events::heartbeat_received e{proxy, heartbeat};
      state_machine.process_event(e);
    }

    void on_gap(Gap && gap) {
      GUID_t writer_guid = {receiver.dest_guid_prefix, gap.writer_id};
      WriterProxy * proxy = reader.matched_writer_lookup(writer_guid);
      assert(proxy);
      cmbml::reader_events::gap_received e{proxy, gap};
      state_machine.process_event(e);
    }

    void on_info_destination(InfoDestination && info_dst) {
      if (info_dst.guid_prefix != guid_prefix_unknown) {
        // guid_prefix is pretty big (12 bytes)
        receiver.dest_guid_prefix = info_dst.guid_prefix;
      } else {
        // Set to participant's guid_prefix, which should be the same as our guid_prefix
        receiver.dest_guid_prefix = reader.guid.prefix;
      }
    }

    void on_data(Data && data) {
      cmbml::reader_events::data_received<RTPSReader> e{reader, data, receiver};
      state_machine.process_event(e);
    }

    MessageReceiver receiver;
    RTPSReader reader;
    boost::msm::lite::sm<typename RTPSReader::StateMachineT> state_machine;
  };
}
}

#endif  // CMBML__DDS__READER_HPP_