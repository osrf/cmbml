#ifndef CMBML__DDS__QOS___HPP_
#define CMBML__DDS__QOS___HPP_

namespace cmbml {
namespace dds {

  // TODO
  struct UserDataQosPolicy {
    BOOST_HANA_DEFINE_STRUCT(UserDataQosPolicy,
      (List<Octet>, value)
    );
  };

  // TODO
  struct SubscriptionQos {
  };

  // TODO
  struct PublisherQos {
  };

  // TODO
  struct TopicQos {
  };

}  // namespace dds
}  // namespace cmbml

#endif // CMBML__DDS__QOS___HPP_
