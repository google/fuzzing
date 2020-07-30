#ifndef ASN1_TYPES_TO_DER_H_
#define ASN1_TYPES_TO_DER_H_

#include <vector>

#include "asn1_types.pb.h"
#include "common.h"

namespace asn1_types {

class ASN1TypesToDER {
 public:
  std::vector<uint8_t> EncodeBitString(const BitString& bit_string);
  std::vector<uint8_t> EncodeInteger(const Integer& integer);
  std::vector<uint8_t> EncodeUTCTime(const UTCTime& utc_time);
  std::vector<uint8_t> EncodeGeneralizedTime(
      const GeneralizedTime& generalized_time);

 private:
  void EncodeTagAndLength(const size_t tag,
                          const size_t len,
                          const size_t pos,
                          std::vector<uint8_t>& der);
  void EncodeDefiniteLength(const size_t actual_len,
                            const size_t len_pos,
                            const std::vector<uint8_t>& der);
};

}  // namespace asn1_types

#endif