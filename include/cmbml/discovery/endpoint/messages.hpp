#ifndef CMBML__SEDP_MESSAGES___HPP_
#define CMBML__SEDP_MESSAGES___HPP_

#include <boost/hana/define_struct.hpp>

#include <cmbml/discovery/common.hpp>
#include <cmbml/dds/qos.hpp>

namespace cmbml {
  struct ContentFilterProperty_t {
    BOOST_HANA_DEFINE_STRUCT(ContentFilterProperty_t,
      (std::array<char, 256>, content_filtered_topic_name),
      (std::array<char, 256>, related_topic_name),
      (String, filter_expression),
      (List<String>, expression_parameters)
    );
  };

  // These messages are PL-serializable.
  // That means each field should have an associated enum
  struct SubscriptionBuiltinTopicData {
    BOOST_HANA_DEFINE_STRUCT(SubscriptionBuiltinTopicData,
      (BuiltinTopicKey_t, key),
      (BuiltinTopicKey_t, participant_key),
      (String, topic_name),
      (String, type_name),
      (dds::SubscriptionQos, qos_policies)
    );

    /*
    CMBML__DEFINE_PARAMETER_ID_MAP(SubscriptionBuiltinTopicData,
      // Not sure what the parameter_id of key is!
      (topic_name, ParameterId_t::topic_name),
      (type_name, ParameterId_t::type_name)
      // qos_policies should get flattened
    );
    */
  };

  struct DiscoReaderData {
    BOOST_HANA_DEFINE_STRUCT(DiscoReaderData,
      (ContentFilterProperty_t, content_filter),
      (SubscriptionBuiltinTopicData, subscription_data),
      (ReaderProxyPOD, reader_proxy)
    );

  };

  struct PublicationBuiltinTopicData {
    BOOST_HANA_DEFINE_STRUCT(PublicationBuiltinTopicData,
      (BuiltinTopicKey_t, key),
      (BuiltinTopicKey_t, participant_key),
      (String, topic_name),
      (String, type_name),
      (dds::PublicationQos, qos_policies)
    );

    /*
    CMBML__DEFINE_PARAMETER_ID_MAP(PublicationBuiltinTopicData,
      (topic_name, ParameterId_t::topic_name),
      (type_name, ParameterId_t::type_name),
    );
    */
  };

  struct DiscoWriterData {
    BOOST_HANA_DEFINE_STRUCT(DiscoWriterData,
      (PublicationBuiltinTopicData, publication_data),
      (WriterProxyPOD, writer_proxy)
    );
  };

  struct TopicBuiltinTopicData {
    BOOST_HANA_DEFINE_STRUCT(TopicBuiltinTopicData,
      (BuiltinTopicKey_t, key),
      (String, name),
      (String, type_name),
      (dds::TopicQos, qos_policies)
    );

    /*
    CMBML__DEFINE_PARAMETER_ID_MAP(TopicBuiltinTopicData,
      (name, ParameterId_t::topic_name),
      (type_name, ParameterId_t::type_name),
    );
    */
  };

}  // namespace cmbml

#endif  // CMBML__SEDP_MESSAGES___HPP_
