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

#include "asn1_pdu_to_der.h"

#include "common.h"

#include <limits.h>

namespace asn1_pdu {

// The maximum level of recursion allowed. Values greater than this will just
// fail.
static constexpr size_t kRecursionLimit = 200;

void ASN1PDUToDER::EncodeOverrideLength(const std::string& raw_len,
                                        size_t len_pos) {
  der_.insert(der_.begin() + len_pos, raw_len.begin(), raw_len.end());
}

void ASN1PDUToDER::EncodeIndefiniteLength(size_t len_pos) {
  der_.insert(der_.begin() + len_pos, 0x80);
  // The PDU's value is from |len_pos| to the end of |der_|, so just add an
  // EOC marker to the end.
  der_.push_back(0x00);
  der_.push_back(0x00);
}

void ASN1PDUToDER::EncodeDefiniteLength(size_t actual_len, size_t len_pos) {
  InsertVariableIntBase256(actual_len, len_pos, der_);
  // X.690 (2015), 8.1.3.3: The long-form is used when the length is
  // larger than 127.
  // Note: |len_num_bytes| is not checked here, because it will equal
  // 1 for values [128..255], but those require the long-form length.
  if (actual_len > 127) {
    // See X.690 (2015) 8.1.3.5.
    // Long-form length is encoded as a byte with the high-bit set to indicate
    // the long-form, while the remaining bits indicate how many bytes are used
    // to encode the length.
    size_t len_num_bytes = GetVariableIntLen(actual_len, 256);
    der_.insert(der_.begin() + len_pos, (0x80 | len_num_bytes));
  }
}

void ASN1PDUToDER::EncodeLength(const Length& len,
                                size_t actual_len,
                                size_t len_pos) {
  if (len.has_length_override()) {
    EncodeOverrideLength(len.length_override(), len_pos);
  } else if (len.has_indefinite_form() && len.indefinite_form()) {
    EncodeIndefiniteLength(len_pos);
  } else {
    EncodeDefiniteLength(actual_len, len_pos);
  }
}

void ASN1PDUToDER::EncodeValue(const Value& val) {
  for (const auto& val_ele : val.val_array()) {
    if (recursion_exceeded_) {
      // If the message exceeds the recursion limit, abort processing the
      // protobuf in order to limit uninteresting work.
      return;
    }
    if (val_ele.has_pdu()) {
      EncodePDU(val_ele.pdu());
    } else {
      der_.insert(der_.end(), val_ele.val_bits().begin(),
                  val_ele.val_bits().end());
    }
  }
}

void ASN1PDUToDER::EncodeHighTagNumberForm(uint8_t id_class,
                                           uint8_t encoding,
                                           uint32_t tag_num) {
  // High-tag-number form requires the lower 5 bits of the identifier to be set
  // to 1 (X.690 (2015), 8.1.2.4.1).
  der_.push_back(id_class | encoding | 0x1F);
  // The high-tag-number form base 128 encodes |tag_num| (X.690 (2015), 8.1.2).
  InsertVariableIntBase128(tag_num, der_.size(), der_);
}

void ASN1PDUToDER::EncodeIdentifier(const Identifier& id) {
  // The class comprises the 7th and 8th bit of the identifier (X.690
  // (2015), 8.1.2).
  uint8_t id_class = static_cast<uint8_t>(id.id_class()) << 6;
  // The encoding comprises the 6th bit of the identifier (X.690 (2015), 8.1.2).
  uint8_t encoding = static_cast<uint8_t>(id.encoding()) << 5;

  uint32_t tag_num = id.tag_num().has_high_tag_num()
                         ? id.tag_num().high_tag_num()
                         : id.tag_num().low_tag_num();
  // When the tag number is greater than or equal to 31, encode with a single
  // byte; otherwise, use the high-tag-number form (X.690 (2015), 8.1.2).
  if (tag_num >= 31) {
    EncodeHighTagNumberForm(id_class, encoding, tag_num);
  } else {
    der_.push_back(static_cast<uint8_t>(id_class | encoding | tag_num));
  }
}

void ASN1PDUToDER::EncodePDU(const PDU& pdu) {
  // Artifically limit the stack depth to avoid stack overflow.
  if (depth_ > kRecursionLimit) {
    recursion_exceeded_ = true;
    return;
  }
  ++depth_;
  EncodeIdentifier(pdu.id());
  size_t len_pos = der_.size();
  EncodeValue(pdu.val());
  EncodeLength(pdu.len(), der_.size() - len_pos, len_pos);
  --depth_;
}

std::vector<uint8_t> ASN1PDUToDER::PDUToDER(const PDU& pdu) {
  // Reset the previous state.
  der_.clear();
  depth_ = 0;
  recursion_exceeded_ = false;

  EncodePDU(pdu);
  if (recursion_exceeded_) {
    der_.clear();
  }
  return der_;
}

}  // namespace asn1_pdu