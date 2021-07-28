#include <libp2p/multi/converters/ethernet_converter.hpp>

#include <string>

#include <libp2p/common/hexutil.hpp>
#include <libp2p/common/types.hpp>
#include <libp2p/multi/converters/conversion_error.hpp>
#include <libp2p/multi/multibase_codec/multibase_codec_impl.hpp>
#include <libp2p/multi/uvarint.hpp>
#include <libp2p/outcome/outcome.hpp>

using std::string_literals::operator""s;

namespace libp2p::multi::converters {

  auto EthernetConverter::addressToHex(std::string_view addr)
      -> outcome::result<std::string> {
      int ret;
      unsigned char mac[6];
      std::string addr_str = std::string{ addr };
      ret = sscanf(addr_str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
      if (6 != ret) {
          return ConversionError::INVALID_ADDRESS;
      }
      std::string result;
      result.assign((const char*)mac, 6);
    return result;
  }

}  // namespace libp2p::multi::converters
