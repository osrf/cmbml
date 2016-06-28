#ifndef CMBML__PSM__UDP__PORTS_HPP_
#define CMBML__PSM__UDP__PORTS_HPP_

namespace cmbml {
namespace udp {
  // TODO Implement DomainId
  struct TuningParameters {
    uint32_t dg = 7400;  // DomainId gain
    uint32_t pb = 250;  // Port Base number
    uint32_t pg = 2;  // ParticipantId gain
    uint32_t d0 = 0;
    uint32_t d1 = 10;
    uint32_t d2 = 1;
    uint32_t d3 = 11;
  };

  constexpr uint64_t default_spdp_multicast_port(uint32_t domain_id, const TuningParameters & p) {
    return p.pg + p.dg * domain_id + p.d0;
  }

  constexpr uint64_t default_spdp_multicast_port(uint32_t domain_id) {
    TuningParameters p;
    return default_spdp_multicast_port(domain_id, p);
  }

  constexpr uint64_t default_spdp_unicast_port(
      uint32_t domain_id, uint32_t participant_id, const TuningParameters & p)
  {
    return p.pg + p.dg * domain_id + p.d1 + p.pg * participant_id;
  }
  constexpr uint64_t default_spdp_unicast_port(uint32_t domain_id, uint32_t participant_id)
  {
    TuningParameters p;
    return default_spdp_unicast_port(domain_id, participant_id, p);
  }

  constexpr uint64_t default_user_multicast_port(uint32_t domain_id, const TuningParameters & p)
  {
    return p.pb + p.dg * domain_id + p.d2;
  }

  constexpr uint64_t default_user_multicast_port(uint32_t domain_id) {
    TuningParameters p;
    return default_user_multicast_port(domain_id, p);
  }

  constexpr uint64_t default_user_unicast_port(
      uint32_t domain_id, uint32_t participant_id, const TuningParameters & p)
  {
    return p.pg + p.dg * domain_id + p.d3 + p.pg * participant_id;
  }
  constexpr uint64_t default_user_unicast_port(uint32_t domain_id, uint32_t participant_id) {
    TuningParameters p;
    return default_user_unicast_port(domain_id, participant_id, p);
  }


}
}

#endif  // CMBML__PSM__UDP__PORTS_HPP_
