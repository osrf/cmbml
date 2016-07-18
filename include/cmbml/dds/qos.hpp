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

  // TODO
  struct SubscriptionQos {
  };

  // TODO
  struct PublicationQos {
  };

  // TODO
  struct TopicQos {
  };

}  // namespace dds
}  // namespace cmbml

#endif // CMBML__DDS__QOS___HPP_
