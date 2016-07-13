#ifndef CMBML__DDS__DDS_ENDPOINT_HPP_
#define CMBML__DDS__DDS_ENDPOINT_HPP_

namespace cmbml {
namespace dds {

  // A DDS endpoint is move constructible but not copy constructible.
  class EndpointBase {
  public:
    explicit EndpointBase(GUID_t & g) : guid(g) {}
    EndpointBase(EndpointBase &&) = default;
    GUID_t & guid;
  protected:
    EndpointBase(const EndpointBase &) = delete;
  };

}  // namespace dds
}  // namespace cmbml

#endif  // CMBML__DDS__DDS_ENDPOINT_HPP_
