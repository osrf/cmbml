#ifndef CMBML__WRITER__HPP_
#define CMBML__WRITER__HPP_

#include <cmbml/structure/history.hpp>
// #include <cmbml/writer_state_machine.hpp>
#include <cmbml/message/data.hpp>

namespace cmbml {

  enum class ChangeForReaderStatusKind {
    unsent, unacknowledged, requested, acknowledged, underway
  };

  struct ChangeForReader : CacheChange {
    ChangeForReaderStatusKind status;
    bool is_relevant;
  };


  // ReaderLocator is MoveAssignable and MoveConstructible
  struct ReaderLocator {

    ReaderLocator(bool inline_qos) : expects_inline_qos(inline_qos) {}
    CacheChange pop_next_requested_change();

    // I believe it is most convenient if next_unsent_change has pop semantics:
    // (removes the change from the unsent_changes list and moves it out of the function.)
    CacheChange pop_next_unsent_change();
    List<CacheChange> * requested_changes() const;
    void set_requested_changes(List<SequenceNumber_t> & request_seq_numbers);
    List<CacheChange> * unsent_changes() const;

    void send(const Data & data);
    void send(const Heartbeat & heartbeat);

    // TODO see below note in ReaderProxy about compile-time behavior here
    bool expects_inline_qos;

  private:

    // TODO In practice, store this using a historycache instead?
    List<CacheChange> requested_changes_list;
    List<CacheChange> unsent_changes_list;
    Locator_t locator;
  };


  struct ReaderProxy {
    // move these structs in
    ReaderProxy(GUID_t & remoteReaderGuid,
        bool expectsInlineQos,
        List<Locator_t> & unicastLocatorList,
        List<Locator_t> & multicastLocatorList);

    GUID_t remote_reader_guid;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;

    SequenceNumber_t set_acked_changes();
    // TODO CacheChange or ChangeForReader?
    CacheChange pop_next_requested_change();
    CacheChange pop_next_unsent_change();
    void set_requested_changes(List<SequenceNumber_t> & request_seq_numbers);
    List<ChangeForReader> * unacked_changes() const;
  private:
    List<CacheChange> changes_for_reader;

    List<CacheChange> unsent_changes_list;
    List<CacheChange> requested_changes_list;

    // TODO can we template these booleans? Would need to template the class
    // and StatefulWriter needs to be able to hold a heterogenous container
    // static const bool expects_inline_qos = expectsInlineQos;
    // static const bool is_active = isActive;
    bool expects_inline_qos;
    bool is_active;
  };

  template<
    bool pushMode,
    typename heartbeatPeriod,
    typename nackResponseDelay = DurationT<0, 500*1000*1000>,
    typename nackSuppressionDuration = DurationT<0, 0>
    >
  struct WriterParams {
    static const bool push_mode = pushMode;
    static constexpr Duration_t heartbeat_period = DurationFactory<heartbeatPeriod>();
    static constexpr Duration_t nack_response_delay = DurationFactory<nackResponseDelay>();
    static constexpr Duration_t nack_suppression_duration = DurationFactory<nackSuppressionDuration>();
  };

  template<typename WriterParams, typename EndpointParams>
  struct Writer : Endpoint<EndpointParams>, WriterParams {

  // TODO safer visibility
  HistoryCache writer_cache;
  protected:
    SequenceNumber_t lastChangeSequenceNumber;
  };

  // Forward declare state machine struct
  // struct StatelessWriterMsm;
  template<typename resendDataPeriod, typename ...Params>
  struct StatelessWriter : Writer<Params...> {
    // StatelessWriterMsm state_machine;

    StatelessWriter() {
      //state_machine.configure<Writer<Params...>::reliability_level>();
    }

    void add_reader_locator(ReaderLocator && locator);
    void remove_reader_locator(ReaderLocator * locator);
    void reset_unset_changes();

    static constexpr Duration_t resend_data_period = DurationFactory<resendDataPeriod>();
  private:
    List<ReaderLocator> reader_locators;
  };

  template<typename ...Params>
  struct StatefulWriter : Writer<Params...> {
    void add_matched_reader(ReaderProxy && reader_proxy);
    void remove_matched_reader(ReaderProxy * reader_proxy);
    ReaderProxy lookup_matched_reader(GUID_t reader_guid);
    bool is_acked_by_all(CacheChange & change);
  private:
    List<ReaderProxy> matched_readers;
  };
}

#endif  // CMBML__WRITER__HPP_
