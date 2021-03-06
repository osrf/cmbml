#ifndef CMBML__WRITER__HPP_
#define CMBML__WRITER__HPP_

#include <cassert>
#include <algorithm>
#include <deque>

#include <cmbml/cdr/serialize_anything.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/psm/udp/context.hpp>
#include <cmbml/structure/history.hpp>

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

    // I believe it is most convenient if next_unsent_change has pop semantics:
    // (removes the change from the unsent_changes list and moves it out of the function.)
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
  };

  // ReaderLocator is MoveAssignable and MoveConstructible
  struct ReaderLocator : ReaderCacheAccessor {

    ReaderLocator(bool inline_qos, HistoryCache * cache) : ReaderCacheAccessor(cache),
      expects_inline_qos(inline_qos) {}

    ReaderLocator(Locator_t && loc, bool inline_qos, HistoryCache * cache) :
      locator(loc),
      ReaderCacheAccessor(cache),
      expects_inline_qos(inline_qos) {}

    template<typename T, typename TransportContext = udp::Context>
    void send(T && msg, TransportContext & context) {
      size_t packet_size = get_packet_size(msg);
      Packet<> packet(packet_size);
      serialize(msg, packet);
      // TODO Implement glomming-on of packets during send and wrapping in Message.
      // context.unicast_send(locator, packet.data(), packet.size());
      send(packet, context);
    }
    template<typename TransportContext = udp::Context>
    void send(Packet<> & packet, TransportContext & context) {
      // TODO Implement glomming-on of packets during send and wrapping in Message.
      context.unicast_send(locator, packet.data(), packet.size());
    }

    bool locator_compare(const Locator_t & loc);
    void reset_unsent_changes();

    // TODO see below note in ReaderProxy about compile-time behavior here
    bool expects_inline_qos;

  private:
    Locator_t locator;
  };


  struct ReaderProxy {
    // move these structs in
    ReaderProxy(GUID_t & remoteReaderGuid,
        bool expectsInlineQos,
        List<Locator_t> && unicastLocatorList,
        List<Locator_t> && multicastLocatorList, HistoryCache * cache) :
      remote_reader_guid(remoteReaderGuid), expects_inline_qos(expectsInlineQos),
      unicast_locator_list(unicastLocatorList), multicast_locator_list(multicastLocatorList),
      cache_accessor(cache), writer_cache(cache)
    {
    }

    GUID_t remote_reader_guid;
    List<Locator_t> unicast_locator_list;
    List<Locator_t> multicast_locator_list;

    ChangeForReader pop_next_requested_change();
    ChangeForReader pop_next_unsent_change();
    void set_requested_changes(List<SequenceNumber_t> & request_seq_numbers);
    void add_change_for_reader(ChangeForReader && change);

    // TODO This should wrap a submessage in a Message packet
    template<typename T, typename TransportContext = udp::Context>
    void send(T && msg, TransportContext & context) {
      size_t packet_size = get_packet_size(msg);
      Packet<> packet(packet_size);
      serialize(msg, packet);

      for (const auto & locator : unicast_locator_list) {
        context.unicast_send(locator, packet.data(), packet.size());
      }
      for (const auto & locator : multicast_locator_list) {
        context.multicast_send(locator, packet.data(), packet.size());
      }
    }


    void set_acked_changes(const SequenceNumber_t & seq_num);

    bool expects_inline_qos;
  private:
    SequenceNumber_t highest_acked_seq_num;
    ReaderCacheAccessor cache_accessor;
    HistoryCache * writer_cache;
    // List<ChangeForReader> changes_for_reader;

    // List<ChangeForReader> unsent_changes_list;
    // List<ChangeForReader> requested_changes_list;

    // TODO can we template these booleans? Would need to template the class
    // and StatefulWriter needs to be able to hold a heterogenous container
    // static const bool expects_inline_qos = expectsInlineQos;
    // static const bool is_active = isActive;
    bool is_active;
  };

  template<bool pushMode, typename EndpointParams>
  struct Writer : Endpoint<EndpointParams>{
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
    Duration_t heartbeat_period = {3, 0};
    Duration_t nack_response_delay = {0, 500*1000*1000};
    Duration_t nack_suppression_duration = {0, 0};
    static const bool push_mode = pushMode;
    Count_t heartbeat_count = 0;
  protected:
    SequenceNumber_t last_change_seq_num;
  };

  // Forward declare state machine struct
  template<bool pushMode, typename EndpointParams>
  struct StatelessWriter : Writer<pushMode, EndpointParams> {

    // TODO
    StatelessWriter() {
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

    template<typename T, typename TransportContext = udp::Context>
    void send(T && msg, TransportContext & context) {
      size_t packet_size = get_packet_size(msg);
      Packet<> packet(packet_size);
      serialize(msg, packet);
      // TODO Implement glomming-on of packets during send and wrapping in Message.
      for (auto & reader_locator : reader_locators) {
        //context.unicast_send(reader_locator.locator, packet.data(), packet.size());
        reader_locator.send(packet, context);
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

    // TODO This should wrap a submessage in a Message packet
    template<typename T, typename TransportContext = udp::Context>
    void send(T && msg, TransportContext & context) {
      size_t packet_size = get_packet_size(msg);
      Packet<> packet(packet_size);
      serialize(msg, packet);

      for (auto reader : matched_readers) {
        for (const auto & locator : reader.unicast_locator_list) {
          context.unicast_send(locator, packet.data(), packet.size());
        }
        for (const auto & locator : reader.multicast_locator_list) {
          context.multicast_send(locator, packet.data(), packet.size());
        }
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
