// Copyright 2020 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef PROTO_ASN1_PDU_UNIVERSAL_TYPES_TO_DER_H_
#define PROTO_ASN1_PDU_UNIVERSAL_TYPES_TO_DER_H_

#include <stdint.h>

#include <vector>

#include "asn1_universal_types.pb.h"

namespace asn1_universal_types {

// DER encodes |integer| according to X.690 (2015), 8.3.
// Appends encoded |integer| to |der|.
void Encode(const Integer& integer, std::vector<uint8_t>& der);

// DER encodes |bit_string| according to X.690 (2015), 8.6.
// Appends encoded |bit_string| to |der|.
void Encode(const BitString& bit_string, std::vector<uint8_t>& der);

// DER encodes |utc_time| according to X.690 (2015), 11.8.
// Appends encoded |utc_time| to |der|.
void Encode(const UTCTime& utc_time, std::vector<uint8_t>& der);

// DER encodes |generalized_time| according to X.690 (2015), 11.7.
// Appends encoded |generalized_time| to |der|.
void Encode(const GeneralizedTime& generalized_time, std::vector<uint8_t>& der);

// Converts |timestamp| to a DER-encoded string (i.e. as used by UTCTime and
// GeneralizedTime), according to X.690 (2015), 11.7 / 11.8.
// |use_two_digit_year| controls whether two or four digits will be used for the
// year.
// Note: Although GeneralizedTime supports fractional seconds, they are not
// encoded here, to ensure compatibility with RFC 5280.
void EncodeTimestamp(const google::protobuf::Timestamp& timestamp,
                     bool use_two_digit_year,
                     std::vector<uint8_t>& der);

}  // namespace asn1_universal_types

#endif  // PROTO_ASN1_PDU_UNIVERSAL_TYPES_TO_DER_H_