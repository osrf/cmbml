// Private implementation header for reader classes

#ifndef CMBML__READER_IMPL__HPP_
#define CMBML__READER_IMPL__HPP_

#include <cmbml/reader.hpp>

namespace cmbml {

  template<typename ...Params>
  void StatefulReader<Params...>::add_matched_writer(WriterProxy & writer_proxy) {
  }

  // Why not remove by GUID?
  template<typename ...Params>
  void StatefulReader<Params...>::remove_matched_writer(WriterProxy * writer_proxy) {
  }

  /*
  template<typename ...Params>
  WriterProxy StatefulReader<Params...>::matched_writer_lookup(
      const GUID_t & writer_guid) const
  {
  }
  */

}

#endif  // CMBML__READER_IMPL__HPP_
