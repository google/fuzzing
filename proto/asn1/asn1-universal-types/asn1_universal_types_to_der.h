#ifndef ASN1_TYPES_TO_DER_H_
#define ASN1_TYPES_TO_DER_H_

#include <vector>

#include "asn1_universal_types.pb.h"
#include "common.h"

namespace asn1_universal_types {

class ASN1UniversalTypesToDER {
 public:
  // Encodes |bit_string| according to X.690 (2015), 8.6;
  // returning a vector with the DER encoding.
  std::vector<uint8_t> EncodeBitString(const BitString& bit_string);

  // Encodes |integer| according to X.690 (2015), 8.3;
  // returning a vector with the DER encoding.
  std::vector<uint8_t> EncodeInteger(const Integer& integer);

  // Encodes |utc_time| according to X.690 (2015), 11.8;
  // returning a vector with the DER encoding.
  std::vector<uint8_t> EncodeUTCTime(const UTCTime& utc_time);

  // Encodes |generalized_time| according to X.690 (2015), 11.7;
  // returning a vector with the DER encoding.
  std::vector<uint8_t> EncodeGeneralizedTime(
      const GeneralizedTime& generalized_time);
};

}  // namespace asn1_types

#endif