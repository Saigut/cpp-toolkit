#ifndef LIBP2P_ETHERNET_CONVERTER_HPP
#define LIBP2P_ETHERNET_CONVERTER_HPP

#include <libp2p/outcome/outcome.hpp>

namespace libp2p::multi::converters {

  /**
   * Converts an ethernet part of a multiaddress (a MAC address)
   * to bytes representation as a hex string
   */
  class EthernetConverter {
   public:
    static auto addressToHex(std::string_view addr)
        -> outcome::result<std::string>;
  };

}  // namespace libp2p::multi::converters

#endif  // LIBP2P_ETHERNET_CONVERTER_HPP
