#ifndef ASN1_UNIVERSAL_TYPES_TO_DER_H_
#define ASN1_UNIVERSAL_TYPES_TO_DER_H_

#include <vector>

#include "asn1_universal_types.pb.h"
#include "common.h"
#include <stdint.h>

namespace asn1_universal_types {

// Encodes |bit_string| according to X.690 (2015), 8.6;
// returning a vector with the encoded bytes.
void Encode(const BitString& bit_string, std::vector<uint8_t>& der);

// Encodes |integer| according to X.690 (2015), 8.3;
// returning a vector with the encoded bytes.
void Encode(const Integer& integer, std::vector<uint8_t>& der);

// Encodes |utc_time| according to X.690 (2015), 11.8;
// returning a vector with the encoded bytes.
void Encode(const UTCTime& utc_time, std::vector<uint8_t>& der);

// Encodes |generalized_time| according to X.690 (2015), 11.7;
// returning a vector with the encoded bytes.
void Encode(const GeneralizedTime& generalized_time, std::vector<uint8_t>& der);

}  // namespace asn1_universal_types

#endif