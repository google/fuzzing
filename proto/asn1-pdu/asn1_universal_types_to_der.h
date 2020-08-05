#ifndef PROTO_ASN1_UNIVERSAL_TYPES_TO_DER_H_
#define PROTO_ASN1_UNIVERSAL_TYPES_TO_DER_H_

#include <google/protobuf/util/time_util.h>
#include <stdint.h>

#include <vector>

#include "asn1_universal_types.pb.h"
#include "common.h"

namespace asn1_universal_types {

// DER encodes |bit_string| according to X.690 (2015), 8.6.
// Appends encoded |bit_string| to |der|.
void Encode(const BitString& bit_string, std::vector<uint8_t>& der);

// DER encodes |integer| according to X.690 (2015), 8.3.
// Appends encoded |integer| to |der|.
void Encode(const Integer& integer, std::vector<uint8_t>& der);

// DER encodes |utc_time| according to X.690 (2015), 11.8.
// Appends encoded |utc_time| to |der|.
void Encode(const UTCTime& utc_time, std::vector<uint8_t>& der);

// DER encodes |generalized_time| according to X.690 (2015), 11.7.
// Appends encoded |generalized_time| to |der|.
void Encode(const GeneralizedTime& generalized_time, std::vector<uint8_t>& der);

// Converts |timestamp| to a DER-encoded string (i.e. as used by UTCTime and
// GeneralizedTime), according to X.690 (2015), 11.7 / 11.8. |use_two_digit_year|
// controls whether two or four digits will be used for the year.
void Encode(const google::protobuf::Timestamp& timestamp,
            bool use_two_digit_year,
            std::vector<uint8_t>& der);

}  // namespace asn1_universal_types

#endif