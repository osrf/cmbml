#ifndef CMBML__READER__HPP_
#define CMBML__READER__HPP_

#include <cmbml/history.hpp>

#include <cmbml/reader_state_machine.hpp>

// #include <boost/hana/tuple.hpp>


namespace cmbml {
  enum class ChangeFromWriterStatusKind {
    lost, missing, received, unknown
  };

  struct WriterProxy {
    WriterProxy(GUID_t guid) : remote_writer_guid(guid) {}

    SequenceNumber_t max_available_changes();
    void set_irrelevant_change(SequenceNumber_t seq_num);
    void update_lost_changes(SequenceNumber_t first_available_seq_num);
    void update_missing_changes(SequenceNumber_t last_available_seq_num);
    void set_received_change(SequenceNumber_t seq_num);

  private:
    GUID_t remote_writer_guid;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;
    List<CacheChange> changes_from_writer;
    List<SequenceNumber_t> missing_changes;
  };


  // TODO How to reproduce the pattern of passing a non-type template parameter as a
  // static const member?
  template<
    Duration_t & heartbeatResponseDelay,
    Duration_t & heartbeatSuppressionDuration,
    static bool expectsInlineQos>
  struct ReaderParams {
    static constexpr Duration_t heartbeat_response_delay = heartbeatResponseDelay;
    static constexpr Duration_t heartbeat_suppression_duration = heartbeatSuppressionDuration;
    static const bool expects_inline_qos = expectsInlineQos;
  };


  //TODO decide if inheritance or template-bool for Stateful/Stateless is better...

  template<bool Stateful, typename ReaderParams, typename EndpointParams>
  struct Reader : Endpoint<EndpointParams>, ReaderParams {
    ReaderMsm state_machine;
    Reader() {
      state_machine.configure<Stateful, EndpointParams::reliability_level>();
    }

  protected:
    HistoryCache reader_cache;
  };

  template<typename ...Params>
  using StatelessReader = Reader<false, Params...>;

  // Stateful specialization
  template<typename ...Params>
  struct Reader<true, Params...> : Endpoint<Params...> {
    ReaderMsm state_machine;
    Reader() {
      state_machine.configure<true, Endpoint<Params...>::reliability_level>();
    }

    void add_matched_writer(WriterProxy & writer_proxy);
    // Why not remove by GUID?
    void remove_matched_writer(WriterProxy * writer_proxy);
    WriterProxy matched_writer_lookup(const GUID_t writer_guid) const;
  private:
    List<WriterProxy> matched_writers;
  };

  template<typename... Params>
  using StatefulReader = Reader<true, Params...>;

}

#endif  // CMBML__READER__HPP_
