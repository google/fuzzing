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

#ifndef PROTO_ASN1_PDU_COMMON_H_
#define PROTO_ASN1_PDU_COMMON_H_

#include <stdint.h>

#include <vector>

constexpr uint8_t kAsn1Constructed = 0x20u;
constexpr uint8_t kAsn1Universal = 0u;
constexpr uint8_t kAsn1Application = 0x40u;
constexpr uint8_t kAsn1ContextSpecific = 0x80u;
constexpr uint8_t kAsn1Private = 0xc0u;

// Boolean is UNIVERSAL 1 (X.680 (2015), 8.6, Table 1) and is always primitive
// (X.690 (2015), 8.2.1).
constexpr uint8_t kAsn1Boolean = kAsn1Universal | 0x01u;
// Integer is UNIVERSAL 2 (X.680 (2015), 8.6, Table 1) and is always primitive
// (X.690 (2015), 8.3.1).
constexpr uint8_t kAsn1Integer = kAsn1Universal | 0x02u;
// Bitstring is UNIVERSAL 3 (X.680 (2015), 8.6, Table 1) and is always
// primitive in DER (X.690 (2015), 10.2).
constexpr uint8_t kAsn1Bitstring = kAsn1Universal | 0x03u;
// ObjectIdentifier is UNIVERSAL 6 (X.680 (2015), 8.6, Table 1) and is always primitive
// (X.690 (2015), 8.19.1).
constexpr uint8_t kAsn1ObjectIdentifier = kAsn1Universal | 0x06u;
// UTCTime has tag number 23 (X.680 (2015), 8.6, Table 1) and is always
// primitive in DER encoding (X.690 (2015), 10.2).
constexpr uint8_t kAsn1UTCTime = kAsn1Universal | 0x17u;
// GeneralizedTime has tag number 24 (X.680 (2015), 8.6, Table 1) and is always
// primitive in DER encoding (X.690 (2015), 10.2).
constexpr uint8_t kAsn1Generalizedtime = kAsn1Universal | 0x18u;
// Sequence has tag number 16 (X.680 (2015), 8.6, Table 1) and is always
// consctructed (X.690 (2015), 8.9.1).
constexpr uint8_t kAsn1Sequence = kAsn1Universal | kAsn1Constructed | 0x10u;

// Returns the number of bytes needed to |base| encode |value| into a
// variable-length unsigned integer with no leading zeros.
uint8_t GetVariableIntLen(uint64_t value, size_t base);

// Converts |value| to a base 128, variable-length, big-endian representation
// and inserts the result into into |der| at |pos|.
void InsertVariableIntBase128(uint64_t value,
                              size_t pos,
                              std::vector<uint8_t>& der);

// Converts |value| to a base 256, variable-length, big-endian representation
// and inserts the result into into |der| at |pos|.
void InsertVariableIntBase256(uint64_t value,
                              size_t pos,
                              std::vector<uint8_t>& der);

// Encodes |tag_byte| and |len| into |der| at |pos| according to X.690
// (2015), 8.1.2-8.1.5.
void EncodeTagAndLength(uint8_t tag_byte,
                        size_t len,
                        size_t pos,
                        std::vector<uint8_t>& der);

// Updates the DER-encoded tag in |der| at offset |pos_of_tag| to a single byte
// tag, |tag_byte|.
// If the existing tag contains a high tag number (>= 31, per
// X.590 (2015), 8.1.2.4), the subsequent identifier octets will be removed, so
// that |der| remains a valid DER encoding.
void ReplaceTag(uint8_t tag_byte, size_t pos_of_tag, std::vector<uint8_t>& der);

#endif  // PROTO_ASN1_PDU_COMMON_H_