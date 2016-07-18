#ifndef CMBML__DDS__QOS___HPP_
#define CMBML__DDS__QOS___HPP_

#include <boost/hana/define_struct.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/string.hpp>

#include <cmbml/message/parameter.hpp>

namespace cmbml {
namespace dds {

  // TODO
  struct UserDataQosPolicy {
    BOOST_HANA_DEFINE_STRUCT(UserDataQosPolicy,
      (List<Octet>, value)
    );
    /*
    CMBML__DEFINE_PARAMETER_ID_MAP(UserDataQosPolicy,
      (value, user_data)
    );
    */

    // how to match a type/field with a ParameterID?
    // Mapping the fields as strings is fairly inconvenient.

    /*
    template<typename KeyType>
    constexpr ParameterId_t get_parameter_id(const KeyType & string_key);

    using valueKeyString = ;
    template<>
    constexpr ParameterId_t get_parameter_id(const valueKeyString & string_key) {
      return ParameterId_t::user_data;
    }

    */
  };

  // TODO I think QoS policy structs need "name"

  // TODO Check underlying types and values
  enum struct DurabilityQosKind {
    volatile_durability,
    transient_local,
    transient,
    persistent
  };

  enum struct LivelinessQosKind {
    automatic,
    manual_by_participant,
    manual_by_topic
  };

  enum struct ReliabilityQosKind {
    best_effort,
    reliable
  };

  enum struct OwnershipQosKind {
    shared,
    exclusive
  };

  enum struct DestinationOrderQosKind {
    by_reception,
    by_source
  };

  enum struct PresentationQosKind {
    instance,
    topic,
    group
  };

  enum struct HistoryQosKind {
    keep_last,
    keep_all
  };

  struct LivelinessQosPolicy {
    BOOST_HANA_DEFINE_STRUCT(LivelinessQosPolicy,
      (Duration_t, lease_duration),
      (LivelinessQosKind, kind)
    );
  };

  struct ReliabilityQosPolicy {
    BOOST_HANA_DEFINE_STRUCT(ReliabilityQosPolicy,
      (ReliabilityQosKind, kind),
      (Duration_t, max_blocking_time)
    );
  };

  struct PresentationQosPolicy {
    BOOST_HANA_DEFINE_STRUCT(PresentationQosPolicy,
      (PresentationQosKind, access_scope),
      (bool, coherent_access),
      (bool, ordered_access)
    );
  };


  // We elide the single-field QosPolicy structs from the standard
  struct SubscriptionQos {
    BOOST_HANA_DEFINE_STRUCT(SubscriptionQos,
      (DurabilityQosKind, durability),
      (Duration_t, deadline),
      (Duration_t, latency_budget),
      (LivelinessQosPolicy, liveliness),
      (ReliabilityQosPolicy, reliability),
      (OwnershipQosKind, ownership),
      (DestinationOrderQosKind, destination_order),
      (UserDataQosPolicy, user_data),
      // (TimeBasedFilterQosPolicy, time_based_filter),  // TODO
      (PresentationQosPolicy, presentation),
      (String, partition_name),
      (String, topic_data),
      (String, group_data),
      // (DurabilityServiceQosPolicy, durability_service),  // TODO
      (Duration_t, lifespan)
    );
  };

  // TODO
  struct PublicationQos {
    BOOST_HANA_DEFINE_STRUCT(PublicationQos,
      (DurabilityQosKind, durability),
      // (DurabilityServiceQosPolicy, durability_service),  // TODO
      (Duration_t, deadline),
      (Duration_t, latency_budget),
      (LivelinessQosPolicy, liveliness),
      (ReliabilityQosPolicy, reliability),
      (Duration_t, lifespan),
      (UserDataQosPolicy, user_data),
      // (TimeBasedFilterQosPolicy, time_based_filter),  // TODO
      (OwnershipQosKind, ownership),
      (int32_t, ownership_strength),
      (DestinationOrderQosKind, destination_order),
      (PresentationQosPolicy, presentation),
      (String, partition_name),
      (String, topic_data),
      (String, group_data)
    );
  };

  struct HistoryQosPolicy {
    BOOST_HANA_DEFINE_STRUCT(HistoryQosPolicy,
      (HistoryQosKind, kind),
      (int32_t, depth)
    );
  };

  struct ResourceLimitsQosPolicy {
    BOOST_HANA_DEFINE_STRUCT(ResourceLimitsQosPolicy,
      (int32_t, max_samples),
      (int32_t, max_instances),
      (int32_t, max_samples_per_instance)
    );
  };

  // TODO
  struct TopicQos {
    BOOST_HANA_DEFINE_STRUCT(TopicQos,
      (DurabilityQosKind, durability),
      // (DurabilityServiceQosPolicy, durability_service),  // TODO
      (Duration_t, deadline),
      (Duration_t, latency_budget),
      (LivelinessQosPolicy, liveliness),
      (ReliabilityQosPolicy, reliability),
      (int32_t, transport_priority),
      (Duration_t, lifespan),
      (DestinationOrderQosKind, destination_order),
      (PresentationQosPolicy, presentation),
      (HistoryQosPolicy, history),
      (ResourceLimitsQosPolicy, resource_limits),
      (OwnershipQosKind, ownership),
      (String, topic_data)
    );
  };

}  // namespace dds
}  // namespace cmbml

#endif // CMBML__DDS__QOS___HPP_
