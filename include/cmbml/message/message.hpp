#ifndef CMBML__MESSAGE__HPP_
#define CMBML__MESSAGE__HPP_

#include <boost/hana/define_struct.hpp>
#include <boost/hana/tuple.hpp>

#include <cmbml/types.hpp>
#include <cmbml/message/header.hpp>
#include <cmbml/message/data.hpp>

namespace cmbml {

/*
template<typename ... Submessages>
struct Message {
  BOOST_HANA_DEFINE_STRUCT(Message,
    (Header, header),
    (boost::hana::tuple<Submessages...>, submessages)
  );
};
*/
struct Message{
  BOOST_HANA_DEFINE_STRUCT(Message,
    (Header, header),
    (List<Submessage>, messages)
  );

  Message () {
  }

  explicit Message(const Message & m) {
    // Header is a POD, fine to copy
    header = m.header;
    // Don't copy the messages?
  }
};

}


#endif  // CMBML__MESSAGE__HPP_
