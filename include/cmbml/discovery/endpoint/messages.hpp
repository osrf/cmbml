#ifndef CMBML__SEDP___HPP_
#define CMBML__SEDP___HPP_

#include <boost/hana/define_struct.hpp>

namespace cmbml {

  struct BuiltinTopicKey_t {
    BOOST_HANA_DEFINE_STRUCT(BuiltinTopicKey_t,
      (uint32_t, value[3])
    );
  }

  struct SubscriptionBuiltinTopicData {
    BOOST_HANA_DEFINE_STRUCT(
      (BuiltinTopicKey_t, key),
      (BuiltinTopicKey_t, participant_key),
      (String, topic_name),
      (String, type_name),
      (dds::SubscriptionQos, qos_policies)
    );
  };

  struct PublicationBuiltinTopicData {
    BOOST_HANA_DEFINE_STRUCT(
      (BuiltinTopicKey_t, key),
      (BuiltinTopicKey_t, participant_key),
      (String, topic_name),
      (String, type_name),
      (dds::PublicationQos, qos_policies)
    );
  };

  struct TopicBuiltinTopicData {
    BOOST_HANA_DEFINE_STRUCT(
      (BuiltinTopicKey_t, key),
      (String, name),
      (String, type_name),
      (dds::TopicQos, qos_policies)
    );
  };

  struct DiscoveredReaderData {
    BOOST_HANA_DEFINE_STRUCT(
      (ContentFilterProperty_t, content_filter),
      (SubscriptionBuiltinTopicData, topic_data)
    );
  };

  using DiscoveredWriterData = PublicationBuiltinTopicData;
  using DiscoveredTopicData = TopicBuiltinTopicData;

}  // namespace cmbml

#endif  // CMBML__SEDP___HPP_
