#ifndef ASN1_UNIVERSAL_TYPES_TO_DER_H_
#define ASN1_UNIVERSAL_TYPES_TO_DER_H_

#include <google/protobuf/util/time_util.h>
#include <stdint.h>

#include <vector>

#include "asn1_universal_types.pb.h"
#include "common.h"

namespace asn1_universal_types {

// Appends DER encoded |bit_string| to |der| according to X.690
// (2015), 8.6.
void Encode(const BitString& bit_string, std::vector<uint8_t>& der);

// Appends DER encoded |integer| to |der| according to X.690
// (2015), 8.3.
void Encode(const Integer& integer, std::vector<uint8_t>& der);

// Appends DER encoded |utc_time| to |der| according to X.690
// (2015), 11.8.
void Encode(const UTCTime& utc_time, std::vector<uint8_t>& der);

// Appends DER encoded |generalized_time| to |der| according to X.690
// (2015), 11.7.
void Encode(const GeneralizedTime& generalized_time, std::vector<uint8_t>& der);

// Appends DER encoded |time_stamp| to |der| where |num_fields| determines
// which type of ASN.1 Time is encoded according to X.690 (2015), 11.7 & 11.8.
void Encode(const google::protobuf::Timestamp& time_stamp,
            const uint8_t num_fields,
            std::vector<uint8_t>& der);

}  // namespace asn1_universal_types

#endif