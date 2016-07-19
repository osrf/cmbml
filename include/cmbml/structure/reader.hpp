#ifndef CMBML__READER__HPP_
#define CMBML__READER__HPP_

#include <cmbml/structure/endpoint.hpp>
#include <cmbml/structure/history.hpp>
#include <cmbml/structure/writer_proxy.hpp>
#include <cmbml/message/data.hpp>
#include <cmbml/psm/udp/transport.hpp>
#include <cmbml/utility/executor.hpp>
#include <cmbml/utility/metafunctions.hpp>
#include <cmbml/serialization/serialize_cdr.hpp>

#include <cassert>
#include <map>

// TODO Comb over actions again making sure that the Endpoint's unicast_locator_list is being used

namespace cmbml {

  // Metafunction declaration, implemented in reader_state_machine.hpp
  template<typename ReaderOptions>
  struct SelectReaderStateMachineType;

  struct WriterProxy {
    WriterProxy(
        GUID_t guid,
        List<Locator_t> & unicast_locators,
        List<Locator_t> & multicast_locators) :
      fields{guid, unicast_locators, multicast_locators}
    {}

    WriterProxy(WriterProxyPOD && pod) : fields(pod)
    {}

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

    template<typename TransportT = cmbml::udp::Transport>
    void send(AckNack && acknack, const Participant & p, TransportT & transport) {
      // TODO Need to wrap with a SubmessageHeader and Message...
      acknack.count = ++acknack_count;
      Packet<> packet = p.serialize_with_header(acknack);
      // needs to know which destination to send to (pass a Locator?)
      // XXX This is dubious.
      for (const auto & locator : fields.unicast_locator_list) {
        transport.unicast_send(locator, packet.data(), packet.size());
      }
      for (const auto & locator : fields.multicast_locator_list) {
        transport.multicast_send(locator, packet.data(), packet.size());
      }
    }


  private:
    WriterProxyPOD fields;

    std::map<uint64_t, ChangeFromWriter> changes_from_writer;
    uint32_t acknack_count = 0;

    uint32_t num_missing_changes = 0;
  };

  template<typename ReaderOptions>
  struct RTPSReader : Endpoint<ReaderOptions> {
    static const bool stateful = ReaderOptions::stateful;

    explicit RTPSReader(Participant & p) : Endpoint<ReaderOptions>(p) {
      Entity::guid.entity_id = p.assign_next_entity_id<RTPSReader>();
    }

    // template<typename ...Args, typename std::enable_if_t<stateful> * = nullptr>
    template<typename ...Args>
    void emplace_matched_writer(Args && ...args) {
      matched_writers.emplace_back(std::forward<Args>(args)...);
    }

    // TODO Error code if failed
    //std::enable_if_t<stateful> remove_matched_writer(const GUID_t & guid) {
    void remove_matched_writer(const GUID_t & guid) {
      for (auto it = matched_writers.begin(); it != matched_writers.end(); ++it) {
        if (it->get_guid() == guid) {
          matched_writers.erase(it);
          return;
        }
      }
    }

    // template<typename FunctionT, typename std::enable_if_t<stateful> * = nullptr>
    template<typename FunctionT>
    void for_each_matched_writer(FunctionT && function) {
      std::for_each(
        matched_writers.begin(), matched_writers.end(), function
      );
    }

    // std::enable_if_t<stateful, WriterProxy *>
    WriterProxy * matched_writer_lookup(const GUID_t & writer_guid) {
      for (auto & writer : matched_writers) {
        if (writer.get_guid() == writer_guid) {
          return &writer;
        }
      }
      return nullptr;
    }


    HistoryCache reader_cache;
    static const bool expects_inline_qos = ReaderOptions::expects_inline_qos;

    // gets overridden by Stateful impl
    Duration_t heartbeat_response_delay = {0, 500*1000*1000};
    Duration_t heartbeat_suppression_duration = {0, 0};

    // Provide code for a user-defined entity by default.
    // Built-in entities will have to override this.
    static const EntityKind entity_kind =
      ternary<ReaderOptions::topic_kind == TopicKind_t::with_key,
      EntityKind, EntityKind::user_reader_with_key, EntityKind::user_reader_no_key>::value;

    using StateMachineT = typename SelectReaderStateMachineType<ReaderOptions>::type;

    // these should be disabled for stateless reader
    List<WriterProxy> matched_writers;
  };

}

#endif  // CMBML__READER__HPP_
