#ifndef CMBML__READER__HPP_
#define CMBML__READER__HPP_

// #include <cmbml/behavior/reader_state_machine.hpp>
#include <cmbml/structure/history.hpp>

#include <cassert>
#include <map>

// #include <cmbml/reader_state_machine.hpp>

// #include <boost/hana/tuple.hpp>


namespace cmbml {
  enum class ChangeFromWriterStatus {
    lost, missing, received, unknown
  };

  // TODO Give copy conversions to and from CacheChange objects
  struct ChangeFromWriter : CacheChange {
    ChangeFromWriterStatus status;
    bool is_relevant;
  };

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

  private:
    GUID_t remote_writer_guid;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
    // TODO relationship between CacheChange and this data structure could be clearer
    std::map<uint64_t, ChangeFromWriter> changes_from_writer;
  };


  // TODO How to reproduce the pattern of passing a non-type template parameter as a
  // static const member?
  template<
    bool expectsInlineQos,
    typename heartbeatResponseDelay = DurationT<0, 500*1000*1000>,
    typename heartbeatSuppressionDuration = DurationT<0, 0>>
  struct ReaderParams {
    static const bool expects_inline_qos = expectsInlineQos;
    static constexpr Duration_t heartbeat_response_delay = DurationFactory<heartbeatResponseDelay>();
    static constexpr Duration_t heartbeat_suppression_duration = DurationFactory<heartbeatSuppressionDuration>();
  };


  //TODO decide if inheritance or template-bool for Stateful/Stateless is better...

  // TODO Methods yo
  template<bool Stateful, typename ReaderParams, typename EndpointParams>
  struct Reader : Endpoint<EndpointParams>, ReaderParams {
    Reader() {
      // state_machine.configure<Stateful, EndpointParams::reliability_level>();
    }

    HistoryCache reader_cache;
    static const bool stateful = Stateful;
  };

  template<typename ...Params>
  using StatelessReader = Reader<false, Params...>;

  // Stateful specialization
  template<typename ReaderParams, typename EndpointParams>
  struct Reader<true, ReaderParams, EndpointParams> : Endpoint<EndpointParams>, ReaderParams {
    static const bool stateful = true;
    Reader() {
    }

    void add_matched_writer(WriterProxy && writer_proxy) {
      matched_writers.insert(std::make_pair(writer_proxy.get_guid(), std::move(writer_proxy)));
    }
    // Why not remove by GUID?
    void remove_matched_writer(WriterProxy * writer_proxy) {
      assert(writer_proxy);
      matched_writers.erase(writer_proxy->get_guid());
    }

    WriterProxy * matched_writer_lookup(const GUID_t & writer_guid) {
      if (!matched_writers.count(writer_guid)) {
        return nullptr;
      }
      return &matched_writers.at(writer_guid);
    }

    HistoryCache reader_cache;
  private:
    std::map<GUID_t, WriterProxy, GUIDCompare> matched_writers;
  };

  template<typename... Params>
  using StatefulReader = Reader<true, Params...>;

}

#endif  // CMBML__READER__HPP_
