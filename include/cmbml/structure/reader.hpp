#ifndef CMBML__READER__HPP_
#define CMBML__READER__HPP_

#include <cmbml/structure/history.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/psm/udp/context.hpp>
#include <cmbml/utility/executor.hpp>
#include <cmbml/cdr/serialize_anything.hpp>

#include <cassert>
#include <map>

namespace cmbml {
  // Forward declarations of state machine types
  // TODO organize headers
  template<typename T>
  struct BestEffortStatelessReaderMsm;
  template<typename T>
  struct BestEffortStatefulReaderMsm;
  template<typename T, typename Transport = cmbml::udp::Context>
  struct ReliableStatefulReaderMsm;

  struct WriterProxy {
    WriterProxy(
        GUID_t guid,
        List<Locator_t> & unicast_locators,
        List<Locator_t> & multicast_locators) :
      remote_writer_guid(guid),
      unicast_locator_list(unicast_locators),
      multicast_locator_list(multicast_locators) {}

    const SequenceNumber_t & max_available_changes();
    void set_irrelevant_change(const SequenceNumber_t & seq_num);
    void set_irrelevant_change(const int64_t seq_num);
    void update_lost_changes(const SequenceNumber_t & first_available_seq_num);
    void update_missing_changes(const SequenceNumber_t & last_available_seq_num);
    void set_received_change(const SequenceNumber_t & seq_num);
    const List<ChangeFromWriter> & missing_changes();
    const GUID_t & get_guid();

    // who provides the Context?
    template<typename TransportContext = cmbml::udp::Context>
    void send(AckNack && acknack, TransportContext & context) {
      // TODO Need to wrap with a SubmessageHeader and Message...
      acknack.count = ++acknack_count;
      size_t packet_size = get_packet_size(acknack);
      Packet<> packet(packet_size);

      // AckNack is variable length so we need to "dynamically" allocate the packet
      serialize(acknack, packet);
      // needs to know which destination to send to (pass a Locator?)
      // XXX This is dubious.
      for (const auto & locator : unicast_locator_list) {
        context.unicast_send(locator, packet.data(), packet.size());
      }
      for (const auto & locator : multicast_locator_list) {
        context.multicast_send(locator, packet.data(), packet.size());
      }
    }

  private:
    GUID_t remote_writer_guid;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
    std::map<uint64_t, ChangeFromWriter> changes_from_writer;
    uint32_t acknack_count = 0;
  };

  template<bool Stateful, bool expectsInlineQos, typename EndpointParams>
  struct Reader : Endpoint<EndpointParams> {
    Reader() {
    }

    HistoryCache reader_cache;
    static const bool stateful = Stateful;
    static const bool expects_inline_qos = expectsInlineQos;

    // gets overridden by Stateful impl
    using StateMachineT = BestEffortStatelessReaderMsm<Reader>;
    Duration_t heartbeat_response_delay = {0, 500*1000*1000};
    Duration_t heartbeat_suppression_duration = {0, 0};
  };

  template<bool expectsInlineQos, typename EndpointParams>
  struct StatelessReader : Reader<true, expectsInlineQos, EndpointParams> {
    StatelessReader() {
    }

    void add_matched_writer(WriterProxy && writer_proxy) {
      matched_writers.insert(std::make_pair(writer_proxy.get_guid(), std::move(writer_proxy)));
    }
    // Why not remove by GUID?
    void remove_matched_writer(WriterProxy * writer_proxy) {
      assert(writer_proxy);
      matched_writers.erase(writer_proxy->get_guid());
    }

    template<typename FunctionT>
    void for_each_matched_writer(FunctionT && function) {
      std::for_each(
        matched_writers.begin().second,
        matched_writers.end().second,
        function
      );
    }

    WriterProxy * matched_writer_lookup(const GUID_t & writer_guid) {
      if (!matched_writers.count(writer_guid)) {
        return nullptr;
      }
      return &matched_writers.at(writer_guid);
    }

    HistoryCache reader_cache;

    using StateMachineT = typename std::conditional<
      StatelessReader::reliability_level == ReliabilityKind_t::best_effort,
      BestEffortStatefulReaderMsm<StatelessReader>, ReliableStatefulReaderMsm<StatelessReader>>::type;
  private:
    std::map<GUID_t, WriterProxy, GUIDCompare> matched_writers;
  };

  template<bool expectsInlineQos, typename... Params>
  using StatefulReader = Reader<true, expectsInlineQos, Params...>;

}

#endif  // CMBML__READER__HPP_
