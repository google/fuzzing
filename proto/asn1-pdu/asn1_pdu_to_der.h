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

#ifndef PROTO_ASN1_PDU_ASN1_PDU_TO_DER_H_
#define PROTO_ASN1_PDU_ASN1_PDU_TO_DER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "asn1_pdu.pb.h"

namespace asn1_pdu {

class ASN1PDUToDER {
 public:
  // Encodes |pdu| to DER, returning the encoded bytes of the PDU in
  // |der_|.
  std::vector<uint8_t> PDUToDER(const PDU& pdu);

 private:
  std::vector<uint8_t> der_;

  // Encodes |pdu| to DER and tracks |depth_| to avoid stack overflow
  // for nested pdu's.
  void EncodePDU(const PDU& pdu);

  // Encodes |id| to DER according to X.690 (2015), 8.1.2.
  void EncodeIdentifier(const Identifier& id);

  // Concatinates |id_class|, |encoding|, and |tag| according to DER
  // high-tag-number form rules (X.690 (2015), 8.1.2.4).
  void EncodeHighTagNumberForm(uint8_t id_class,
                               uint8_t encoding,
                               uint32_t tag);

  // Encodes the length to DER.
  // |len| can be used to affect the encoding, in order to produce
  // invalid lengths. |actual_len| is the correct length of the PDU,
  // and is used when |len| is not. |len_pos| contains the offset in |der_|
  // where the length should be encoded.
  // To correctly call this, the tag must already be encoded immediately prior
  // to |len_pos|, and the remainder of |der_| represents the encoded value.
  void EncodeLength(const Length& len, size_t actual_len, size_t len_pos);

  // Writes |raw_len| to |der_| at |len_pos|.
  void EncodeOverrideLength(const std::string& raw_len, size_t len_pos);

  // Encodes the indefinite-length indicator (X.690 (2015), 8.1.3.6) at
  // |len_pos|, and appends an End-of-Contents (EOC) marker at the end of
  // |der_|.
  void EncodeIndefiniteLength(size_t len_pos);

  // Encodes the length in |actual_len| using the definite-form length (X.690
  // (2015), 8.1.3-8.1.5 & 10.1) into |der_| at |len_pos|.
  void EncodeDefiniteLength(size_t actual_len, size_t len_pos);

  // Extracts bytes from |val| and inserts them into |enocder_|.
  void EncodeValue(const Value& val);

  // Tracks recursion depth to avoid stack exhaustion.
  size_t depth_;

  // Whether |depth_| exceeded the recursion limit, causing an early return.
  bool recursion_exceeded_;
};

}  // namespace asn1_pdu

#endif  // PROTO_ASN1_PDU_ASN1_PDU_TO_DER_H_