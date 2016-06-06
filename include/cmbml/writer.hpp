#ifndef CMBML__WRITER__HPP_
#define CMBML__WRITER__HPP_

#include <cmbml/history.hpp>
#include <cmbml/writer_state_machine.hpp>

namespace cmbml {

  enum class ChangeForReaderStatusKind {
    unsent, unacknowledged, requested, acknowledged, underway
  };

  struct ChangeForReader {
    ChangeForReaderStatusKind status;
    bool is_relevant;
  };

  struct ReaderLocator {

    ReaderLocator(bool inline_qos) : expects_inline_qos(inline_qos) {}
    ChangeForReader * next_requested_change() const;
    ChangeForReader * next_unsent_change() const;
    List<CacheChange> * requested_changes() const;
    void set_requested_changes(List<SequenceNumber_t> & request_seq_numbers);
    List<CacheChange> * unsent_changes() const;

  private:
    // TODO see below note in ReaderProxy about compile-time behavior here
    bool expects_inline_qos;

    List<CacheChange> requested_changes_list;
    List<CacheChange> unsent_changes_list;
    Locator_t locator;
  };


  struct ReaderProxy {
    GUID_t remote_reader_guid;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;

    SequenceNumber_t set_acked_changes();
    ChangeForReader * next_requested_change() const;
    ChangeForReader * next_unsent_change() const;
    List<ChangeForReader> * unsent_changes() const;
    List<ChangeForReader> * requested_changes() const;
    void set_requested_changes(List<SequenceNumber_t> & request_seq_numbers);
    List<ChangeForReader> * unacked_changes() const;
  private:
    List<CacheChange> changes_for_reader;
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

  protected:
    SequenceNumber_t lastChangeSequenceNumber;
    HistoryCache writer_cache;
  };

  template<typename resendDataPeriod, typename ...Params>
  struct StatelessWriter : Writer<Params...> {
    StatelessWriterMsm state_machine;

    StatelessWriter() {
      // TODO
      // auto state_machine = configure_state_machine(this);
      state_machine.configure<Writer<Params...>::reliability_level>();
    }

    // TODO: can we move the locator?
    void add_reader_locator(Locator_t & locator);
    void remove_reader_locator(Locator_t * locator);
    void reset_unset_changes();

    static constexpr Duration_t resend_data_period = DurationFactory<resendDataPeriod>();
  private:
    List<ReaderLocator> reader_locators;
  };

  template<typename ...Params>
  struct StatefulWriter : Writer<Params...> {
    void add_matched_reader(ReaderProxy & reader_proxy);
    void remove_matched_reader(ReaderProxy * reader_proxy);
    ReaderProxy lookup_matched_reader(GUID_t reader_guid);
    bool is_acked_by_all(CacheChange & change);
  private:
    List<ReaderProxy> matched_readers;
  };
}

#endif  // CMBML__WRITER__HPP_
