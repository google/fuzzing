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

#include "common.h"

#include <limits.h>
#include <math.h>

uint8_t GetVariableIntLen(uint64_t value, size_t base) {
  uint8_t base_bits = log2(base);
  for (uint8_t num_bits = (sizeof(value) - 1) * CHAR_BIT; num_bits >= base_bits;
       num_bits -= base_bits) {
    if (value >> num_bits) {
      return ceil(static_cast<double>(num_bits) / base_bits) + 1;
    }
  }
  // Special-case: zero requires one, not zero bytes.
  return 1;
}

void InsertVariableIntBase128(uint64_t value,
                              size_t pos,
                              std::vector<uint8_t>& der) {
  std::vector<uint8_t> variable_int;
  for (uint8_t i = GetVariableIntLen(value, 128) - 1; i != 0; --i) {
    // If it's not the last byte, the high bit is set to 1.
    variable_int.push_back((0x01 << 7) | ((value >> (i * 7)) & 0x7F));
  }
  variable_int.push_back(value & 0x7F);
  der.insert(der.begin() + pos, variable_int.begin(), variable_int.end());
}

void InsertVariableIntBase256(uint64_t value,
                              size_t pos,
                              std::vector<uint8_t>& der) {
  std::vector<uint8_t> variable_int;
  for (uint8_t shift = GetVariableIntLen(value, 256); shift != 0; --shift) {
    variable_int.push_back((value >> ((shift - 1) * CHAR_BIT)) & 0xFF);
  }
  der.insert(der.begin() + pos, variable_int.begin(), variable_int.end());
}

void EncodeTagAndLength(uint8_t tag_byte,
                        size_t len,
                        size_t pos,
                        std::vector<uint8_t>& der) {
  InsertVariableIntBase256(len, pos, der);
  // X.690 (2015), 8.1.3.3: The long-form is used when the length is
  // larger than 127.
  // Note: |len_num_bytes| is not checked here, because it will equal
  // 1 for values [128..255], but those require the long-form length.
  if (len > 127) {
    // See X.690 (2015) 8.1.3.5.
    // Long-form length is encoded as a byte with the high-bit set to indicate
    // the long-form, while the remaining bits indicate how many bytes are used
    // to encode the length.
    size_t len_num_bytes = GetVariableIntLen(len, 256);
    der.insert(der.begin() + pos, (0x80 | len_num_bytes));
  }
  der.insert(der.begin() + pos, tag_byte);
}

void ReplaceTag(uint8_t tag_byte,
                size_t pos_of_tag,
                std::vector<uint8_t>& der) {
  if (der.size() <= pos_of_tag) {
    return;
  }
  auto start = der.begin() + pos_of_tag;
  auto end = start + 1;
  // Check to see if it's a high tag (multi-byte)
  if ((*start & 0x1F) == 0x1F) {
    // High-tag will have 0x80 set if there is >1 byte remaining
    while (end != der.end() && (*end & 0x80))
      ++end;
    start = der.erase(start, end);
  }
  *start = tag_byte;
}