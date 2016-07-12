#ifndef CMBML__READER__HPP_
#define CMBML__READER__HPP_

#include <cmbml/structure/endpoint.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/psm/udp/context.hpp>
#include <cmbml/utility/executor.hpp>
#include <cmbml/utility/metafunctions.hpp>
#include <cmbml/cdr/serialize_anything.hpp>

#include <cassert>
#include <map>

// TODO Comb over actions again making sure that the Endpoint's unicast_locator_list is being used

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

    SequenceNumber_t max_available_changes();
    void set_irrelevant_change(const SequenceNumber_t & seq_num);
    void set_irrelevant_change(const int64_t seq_num);
    void update_lost_changes(const SequenceNumber_t & first_available_seq_num);
    void update_missing_changes(const SequenceNumber_t & last_available_seq_num);
    void set_received_change(const SequenceNumber_t & seq_num);

    template<typename Function>
    void for_each_missing_change(Function && function) {
      for (auto & change_pair : changes_from_writer) {
        ChangeFromWriter & change = change_pair.second;
        if (change.status == ChangeFromWriterStatus::missing) {
          function(change);
        }
      }
    }

    const GUID_t & get_guid();
    void update_missing_changes_count(
      const ChangeFromWriter & change, ChangeFromWriterStatus future_status);

    bool missing_changes_empty = true;
    bool missing_changes_not_empty = false;

    // who provides the Context?
    template<typename TransportContext = cmbml::udp::Context>
    void send(AckNack && acknack, const Participant & p, TransportContext & context) {
      // TODO Need to wrap with a SubmessageHeader and Message...
      acknack.count = ++acknack_count;
      Packet<> packet = p.serialize_with_header(acknack);
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

    uint32_t num_missing_changes = 0;
  };

  template<bool Stateful, bool expectsInlineQos, typename EndpointParams>
  struct Reader : Endpoint<EndpointParams> {
    explicit Reader(Participant & p) : Endpoint<EndpointParams>(p) {
      Entity::guid.entity_id = p.assign_next_entity_id<Reader>();
    }

    HistoryCache reader_cache;
    static const bool stateful = Stateful;
    static const bool expects_inline_qos = expectsInlineQos;

    // gets overridden by Stateful impl
    using StateMachineT = BestEffortStatelessReaderMsm<Reader>;
    Duration_t heartbeat_response_delay = {0, 500*1000*1000};
    Duration_t heartbeat_suppression_duration = {0, 0};

    // Provide code for a user-defined entity by default.
    // Built-in entities will have to override this.
    static const EntityKind entity_kind = ternary<
      EndpointParams::topic_kind == TopicKind_t::with_key, EntityKind,
      EntityKind::user_reader_with_key, EntityKind::user_reader_no_key>::value;
  };

  template<bool expectsInlineQos, typename EndpointParams>
  struct StatefulReader : Reader<true, expectsInlineQos, EndpointParams> {
    explicit StatefulReader(Participant & p) : Reader<true, expectsInlineQos, EndpointParams>(p) {
    }

    template<typename ...Args>
    void emplace_matched_writer(Args && ...args) {
      matched_writers.emplace_back(std::forward<Args>(args)...);
    }

    // TODO Error code if failed
    void remove_matched_writer(const GUID_t & guid) {
      for (auto it = matched_writers.begin(); it != matched_writers.end(); ++it) {
        if (it->get_guid() == guid) {
          matched_writers.erase(it);
          return;
        }
      }
    }

    template<typename FunctionT>
    void for_each_matched_writer(FunctionT && function) {
      std::for_each(
        matched_writers.begin(), matched_writers.end(), function
      );
    }

    WriterProxy * matched_writer_lookup(const GUID_t & writer_guid) {
      for (auto & writer : matched_writers) {
        if (writer.get_guid() == writer_guid) {
          return &writer;
        }
      }
      return nullptr;
    }

    HistoryCache reader_cache;

    using StateMachineT = typename std::conditional<
      StatefulReader::reliability_level == ReliabilityKind_t::best_effort,
      BestEffortStatefulReaderMsm<StatefulReader>, ReliableStatefulReaderMsm<StatefulReader>>::type;
  private:
    List<WriterProxy> matched_writers;
  };

  template<bool expectsInlineQos, typename... Params>
  using StatelessReader = Reader<true, expectsInlineQos, Params...>;

}

#endif  // CMBML__READER__HPP_
