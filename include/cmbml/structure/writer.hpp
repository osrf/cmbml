#ifndef CMBML__WRITER__HPP_
#define CMBML__WRITER__HPP_

#include <algorithm>
#include <cassert>
#include <chrono>
#include <deque>
#include <utility>

#include <cmbml/structure/endpoint.hpp>
#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/psm/udp/context.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/utility/executor.hpp>
#include <cmbml/utility/metafunctions.hpp>

// TODO Comb over actions again making sure that the Endpoint's unicast_locator_list is being used

namespace cmbml {

  // Metafunction declaration, implemented in writer_state_machine.hpp
  template<typename WriterOptions>
  struct SelectWriterStateMachineType;

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

    bool unsent_changes_empty = true;
    // TODO There are no hooks through ReaderLocator that set unsent_changes_not_empty
    bool unsent_changes_not_empty = false;
    bool requested_changes_empty = true;
    bool requested_changes_not_empty = false;
  };

  // ReaderLocator is MoveAssignable and MoveConstructible
  struct ReaderLocator : ReaderCacheAccessor {

    ReaderLocator(bool inline_qos, HistoryCache * cache) : ReaderCacheAccessor(cache),
      expects_inline_qos(inline_qos) {}

    ReaderLocator(Locator_t && loc, bool inline_qos, HistoryCache * cache) :
      locator(loc),
      ReaderCacheAccessor(cache),
      expects_inline_qos(inline_qos) {}

    bool operator==(const ReaderLocator & loc);
    bool key_matches(const Locator_t & loc);

    void reset_unsent_changes();

    // TODO see below note in ReaderProxy about compile-time behavior here
    bool expects_inline_qos;
    const Locator_t & get_locator() const {
      return locator;
    };

    bool can_send;
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
    bool operator==(const ReaderProxy & proxy);
    bool key_matches(const GUID_t & guid);

    const bool expects_inline_qos;

    bool unacked_changes_not_empty = false;
    bool unacked_changes_empty = true;
    bool can_send = false;
  private:
    SequenceNumber_t highest_acked_seq_num;
    HistoryCache * writer_cache;

    bool is_active = false;  // hmm

    uint32_t num_unacked_changes = 0;
  };


  template<typename WriterOptions>
  struct RTPSWriter : Endpoint<WriterOptions> {
    static const bool push_mode = WriterOptions::push_mode;
    static const bool stateful = WriterOptions::stateful;

    using MatchedReader = std::conditional_t<stateful, ReaderProxy, ReaderLocator>;
    using ReaderKey = std::conditional_t<stateful, GUID_t, Locator_t>;

    explicit RTPSWriter(Participant & p) :
      Endpoint<WriterOptions>(p)
    {
      Entity::guid.entity_id = p.assign_next_entity_id<RTPSWriter>();
    }

    template<typename ...Args>
    void emplace_matched_reader(Args && ...args) {
      matched_readers.emplace_back(std::forward<Args>(args)...);
    }

    void remove_matched_reader(MatchedReader * reader_proxy) {
      assert(reader_proxy);
      for (auto it = matched_readers.begin(); it != matched_readers.end(); ++it) {
        if (*it == *reader_proxy) {
          matched_readers.erase(it);
          return;
        }
      }
    }

    // Precondition: reader is in matched_readers
    MatchedReader & lookup_matched_reader(ReaderKey & key) {
      for (auto & reader : matched_readers) {
        if (reader.key_matches(key)) {
          return reader;
        }
      }
      assert(false);
    }

    template<typename FunctionT>
    void for_each_matched_reader(FunctionT && function) {
      for (auto & reader : matched_readers) {
        function(reader);
      }
    }

    CacheChange new_change(ChangeKind_t k, SerializedData && data, InstanceHandle_t & handle) {
      auto ret = CacheChange(k, std::move(data), handle, this->guid);
      ret.sequence_number = writer_cache.get_max_sequence_number() + 1;
      return ret;
    }

    CacheChange new_change(ChangeKind_t k, InstanceHandle_t & handle) {
      auto ret = CacheChange(k, handle, this->guid);
      ret.sequence_number = writer_cache.get_max_sequence_number() + 1;
      return ret;
    }

    void add_change(ChangeKind_t k, SerializedData && data, InstanceHandle_t & handle) {
      writer_cache.add_change(std::move(new_change(k, std::move(data), handle)));
      conditionally_execute<!stateful>::call(
        [](auto & reader_locators) {
          for (auto & reader_locator : reader_locators) {
            if (reader_locator.num_unsent_changes == 0) {
              reader_locator.unsent_changes_not_empty = true;
            }
            ++reader_locator.num_unsent_changes;
          }
        }, matched_readers);
    }

    void add_change(ChangeKind_t k, InstanceHandle_t & handle) {
      writer_cache.add_change(std::move(new_change(k, handle)));
      conditionally_execute<!stateful>::call(
        [](auto & reader_locators) {
          for (auto & reader_locator : reader_locators) {
            if (reader_locator.num_unsent_changes == 0) {
              reader_locator.unsent_changes_not_empty = true;
            }
            ++reader_locator.num_unsent_changes;
          }
        }, matched_readers);
    }

    // Stateless-specific
    std::enable_if_t<!stateful> reset_unsent_changes() {
      for (auto & reader : matched_readers) {
        reader.reset_unsent_changes();
      }
    }

    template<typename T, typename TransportContext = udp::Context,
      typename std::enable_if_t<!stateful> * = nullptr>
    void send_to_all_locators(T && msg, TransportContext & context) {
      Packet<> packet = Endpoint<WriterOptions>::participant.serialize_with_header(msg);
      // TODO Implement glomming-on of packets during send and wrapping in Message.
      // TODO Check if the locator is unicast or multicast before sending
      //
    }

    template<typename T, typename TransportContext = udp::Context>
    void send_to_all_locators(T && msg, TransportContext & context) {
      Packet<> packet = Endpoint<WriterOptions>::participant.serialize_with_header(msg);

      conditionally_execute<stateful>::call(
        [&context, &packet](auto & readers) {
          for (auto & reader : readers) {
            for (const auto & locator : reader.unicast_locator_list) {
              context.unicast_send(locator, packet.data(), packet.size());
            }
            for (const auto & locator : reader.multicast_locator_list) {
              context.multicast_send(locator, packet.data(), packet.size());
            }
          }
        }, matched_readers);
      conditionally_execute<!stateful>::call(
        [&context, &packet](auto & readers) {
          for (auto & reader_locator : readers) {
            context.unicast_send(reader_locator.get_locator(), packet.data(), packet.size());
          }
        }, matched_readers);
    }

    HistoryCache writer_cache;
    Duration_t nack_response_delay = {0, 500*1000*1000};
    Duration_t heartbeat_period = {3, 0};
    Duration_t nack_suppression_duration = {0, 0};

    // only applies to stateful
    Duration_t resend_data_period = {3, 0};

    static const EntityKind entity_kind = ternary<
      WriterOptions::topic_kind == TopicKind_t::with_key, EntityKind,
      EntityKind::user_writer_with_key, EntityKind::user_writer_no_key>::value;

    using StateMachineT = typename SelectWriterStateMachineType<WriterOptions>::type;

    Count_t heartbeat_count = 0;
  private:
    List<MatchedReader> matched_readers;
    SequenceNumber_t last_change_seq_num;
  };

  // ACTUALLY we could template the Writer on the Reader Type
  // Because the readers and writers have effectively the same interface
  // Then these specializations could be selected based on the "QoS" parameters

}

#endif  // CMBML__WRITER__HPP_
