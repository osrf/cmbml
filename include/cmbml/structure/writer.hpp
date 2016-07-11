#ifndef CMBML__WRITER__HPP_
#define CMBML__WRITER__HPP_

#include <cassert>
#include <chrono>
#include <algorithm>
#include <deque>

#include <cmbml/structure/endpoint.hpp>
#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/psm/udp/context.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/utility/executor.hpp>
#include <cmbml/utility/metafunctions.hpp>

// TODO Comb over actions again making sure that the Endpoint's unicast_locator_list is being used

namespace cmbml {
  // Forward declarations of state machine types.

  template<typename T>
  struct BestEffortStatelessWriterMsm;
  template<typename T>
  struct ReliableStatelessWriterMsm;
  template<typename T>
  struct BestEffortStatefulWriterMsm;
  template<typename T>
  struct ReliableStatefulWriterMsm;

  struct ReaderCacheAccessor {
    ReaderCacheAccessor(HistoryCache * cache) : writer_cache(cache) {
    }

    CacheChange pop_next_requested_change();

    CacheChange pop_next_unsent_change();

    void set_requested_changes(const List<SequenceNumber_t> & request_seq_numbers);

    // Probably more efficient to store as a uint64_t here
    SequenceNumber_t highest_seq_num_sent = {0, 0};
    SequenceNumber_t lowest_requested_seq_num;
    // Ideally, this would be ordered
    // TODO Yeah, I think we need to change this to enforce order
    std::deque<SequenceNumber_t> requested_seq_num_set;
    // TODO In order to make multithreading safe, how to express synchronization between readers?
    HistoryCache * writer_cache;

    uint32_t num_unsent_changes = 0;
    uint32_t num_requested_changes = 0;

    dds::GuardCondition unsent_changes_empty;
    // TODO There are no hooks through ReaderLocator that set unsent_changes_not_empty
    dds::GuardCondition unsent_changes_not_empty;
    dds::GuardCondition requested_changes_empty;
    dds::GuardCondition requested_changes_not_empty;
  };

  // ReaderLocator is MoveAssignable and MoveConstructible
  struct ReaderLocator : ReaderCacheAccessor {

    ReaderLocator(bool inline_qos, HistoryCache * cache) : ReaderCacheAccessor(cache),
      expects_inline_qos(inline_qos) {}

    ReaderLocator(Locator_t && loc, bool inline_qos, HistoryCache * cache) :
      locator(loc),
      ReaderCacheAccessor(cache),
      expects_inline_qos(inline_qos) {}


    bool locator_compare(const Locator_t & loc);
    void reset_unsent_changes();

    // TODO see below note in ReaderProxy about compile-time behavior here
    bool expects_inline_qos;
    const Locator_t & get_locator() const {
      return locator;
    };

    dds::GuardCondition can_send;
  private:
    Locator_t locator;
  };


  struct ReaderProxy : ReaderCacheAccessor {
    // move these structs in
    ReaderProxy(GUID_t & remoteReaderGuid,
        bool expectsInlineQos,
        List<Locator_t> && unicastLocatorList,
        List<Locator_t> && multicastLocatorList, HistoryCache * cache) :
      remote_reader_guid(remoteReaderGuid), expects_inline_qos(expectsInlineQos),
      unicast_locator_list(unicastLocatorList), multicast_locator_list(multicastLocatorList),
      writer_cache(cache), ReaderCacheAccessor(cache)
    {
    }

    GUID_t remote_reader_guid;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;

    ChangeForReader pop_next_requested_change();
    ChangeForReader pop_next_unsent_change();
    void set_requested_changes(List<SequenceNumber_t> & request_seq_numbers);
    void add_change_for_reader(ChangeForReader && change);

    void set_acked_changes(const SequenceNumber_t & seq_num);

    bool expects_inline_qos;
    dds::GuardCondition unacked_changes_not_empty;
    dds::GuardCondition unacked_changes_empty;
    dds::GuardCondition can_send;
  private:
    SequenceNumber_t highest_acked_seq_num;
    // ReaderCacheAccessor cache_accessor;
    HistoryCache * writer_cache;

    // static const bool is_active = isActive;
    bool is_active;

    uint32_t num_unacked_changes = 0;
  };

  template<bool pushMode, typename EndpointParams>
  struct Writer : Endpoint<EndpointParams> {

    explicit Writer(Participant & p) : Endpoint<EndpointParams>(p) {
      Entity::guid.entity_id = p.assign_next_entity_id<Writer>();
    }

    CacheChange new_change(ChangeKind_t k, Data && data, InstanceHandle_t && handle) {
      auto ret = CacheChange(k, data, handle, this->guid);
      ret.sequence_number = writer_cache.get_max_sequence_number() + 1;
      return ret;
    }

    CacheChange new_change(ChangeKind_t k, InstanceHandle_t && handle) {
      auto ret = CacheChange(k, handle, this->guid);
      ret.sequence_number = writer_cache.get_max_sequence_number() + 1;
      return ret;
    }

    void add_change(ChangeKind_t k, Data && data, InstanceHandle_t && handle) {
      writer_cache.add_change(std::move(new_change(k, data, handle)));
    }

    void add_change(ChangeKind_t k, InstanceHandle_t && handle) {
      writer_cache.add_change(std::move(new_change(k, handle)));
    }

    HistoryCache writer_cache;
    Duration_t nack_response_delay = {0, 500*1000*1000};
    Duration_t heartbeat_period = {3, 0};
    Duration_t nack_suppression_duration = {0, 0};
    static const bool push_mode = pushMode;

    static const EntityKind entity_kind = ternary<
      EndpointParams::topic_kind == TopicKind_t::with_key, EntityKind,
      EntityKind::user_writer_with_key, EntityKind::user_writer_no_key>::value;

    Count_t heartbeat_count = 0;
  protected:
    SequenceNumber_t last_change_seq_num;
  };

  // Forward declare state machine struct
  template<bool pushMode, typename EndpointParams>
  struct StatelessWriter : Writer<pushMode, EndpointParams> {
    using ParentWriter = Writer<pushMode, EndpointParams>;
    explicit StatelessWriter(Participant & p) : Writer<pushMode, EndpointParams>(p) {
      // TODO Instantiate reader locators based on participant defaults?
    }

    void add_reader_locator(ReaderLocator && locator) {
      reader_locators.push_back(locator);
    }

    // TODO Better identifier?
    void remove_reader_locator(ReaderLocator * locator) {
      assert(locator);
      std::remove_if(reader_locators.begin(), reader_locators.end(),
        [locator](auto x) {
          return &x == locator;
        });
    }

    ReaderLocator & lookup_reader_locator(Locator_t & locator) {
      for (auto & reader_locator : reader_locators) {
        if (reader_locator.locator_compare(locator)) {
          return reader_locator;
        }
      }
      assert(false);
    }

    void reset_unsent_changes() {
      for (auto & reader : reader_locators) {
        reader.reset_unsent_changes();
      }
    }

    void add_change(ChangeKind_t k, Data && data, InstanceHandle_t && handle) {
      ParentWriter::add_change(k, data, handle);
      // writer_cache.add_change(std::move(new_change(k, data, handle)));
      for (auto & reader_locator : reader_locators) {
        if (reader_locator.num_unsent_changes == 0) {
          reader_locator.unsent_changes_not_empty.set_trigger_value();
        }
        ++reader_locator.num_unsent_changes;
      }
    }

    void add_change(ChangeKind_t k, InstanceHandle_t && handle) {
      ParentWriter::add_change(k, handle);
      // writer_cache.add_change(std::move(new_change(k, handle)));
      for (auto & reader_locator : reader_locators) {
        if (reader_locator.num_unsent_changes == 0) {
          reader_locator.unsent_changes_not_empty.set_trigger_value();
        }
        ++reader_locator.num_unsent_changes;
      }
    }

    template<typename FunctionT>
    void for_each_matched_reader(FunctionT && function) {
      for (auto & locator : reader_locators) {
        function(locator);
      }
    }

    template<typename T, typename TransportContext = udp::Context>
    void send_to_all_locators(T && msg, TransportContext & context) {
      Packet<> packet = Endpoint<EndpointParams>::participant.serialize_with_header(msg);
      // TODO Implement glomming-on of packets during send and wrapping in Message.
      // TODO Check if the locator is unicast or multicast before sending
      for (auto & reader_locator : reader_locators) {
        context.unicast_send(reader_locator.get_locator(), packet.data(), packet.size());
      }
    }

    static const bool stateful = false;
    using StateMachineT = typename std::conditional<
      StatelessWriter::reliability_level == ReliabilityKind_t::best_effort,
      BestEffortStatelessWriterMsm<StatelessWriter>, ReliableStatelessWriterMsm<StatelessWriter>>::type;
  private:
    List<ReaderLocator> reader_locators;
  };

  template<bool pushMode, typename EndpointParams>
  struct StatefulWriter : Writer<pushMode, EndpointParams> {
    explicit StatefulWriter(Participant & p) : Writer<pushMode, EndpointParams>(p) {
      // TODO Instantiate readerproxies based on participant defaults?
    }

    void add_matched_reader(ReaderProxy && reader_proxy) {
      matched_readers.push_back(reader_proxy);
    }

    void remove_matched_reader(ReaderProxy * reader_proxy) {
      assert(reader_proxy);
      std::remove_if(matched_readers.begin(), matched_readers.end(),
        [reader_proxy](auto & reader) {
          return reader.remote_reader_guid == reader_proxy->remote_reader_guid;
        }
      );
    }

    ReaderProxy & lookup_matched_reader(const GUID_t & reader_guid) {
      for (auto & reader : matched_readers) {
        if (reader.remote_reader_guid == reader_guid) {
          return reader;
        }
      }
      assert(false);
    }

    // TODO
    bool is_acked_by_all(CacheChange & change);

    template<typename T, typename TransportContext = udp::Context>
    void send_to_all_locators(T && msg, TransportContext & context) {
      Packet<> packet = Endpoint<EndpointParams>::participant.serialize_with_header(msg);

      for (auto reader : matched_readers) {
        for (const auto & locator : reader.unicast_locator_list) {
          context.unicast_send(locator, packet.data(), packet.size());
        }
        for (const auto & locator : reader.multicast_locator_list) {
          context.multicast_send(locator, packet.data(), packet.size());
        }
      }
    }

    template<typename CallbackT>
    void for_each_matched_reader(CallbackT && callback) {
      for (auto & reader : matched_readers) {
        callback(reader);
      }
    }


    // TODO is this default reasonable? (not in the spec)
    Duration_t resend_data_period = {3, 0};
    static const bool stateful = true;
    using StateMachineT = typename std::conditional<
      StatefulWriter::reliability_level == ReliabilityKind_t::best_effort,
      BestEffortStatefulWriterMsm<StatefulWriter>, ReliableStatefulWriterMsm<StatefulWriter>>::type;

  private:
    List<ReaderProxy> matched_readers;
  };

  // ACTUALLY we could template the Writer on the Reader Type
  // Because the readers and writers have effectively the same interface
  // Then these specializations could be selected based on the "QoS" parameters

}

#endif  // CMBML__WRITER__HPP_
