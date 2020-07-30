#include "common.h"

uint8_t GetVariableIntLen(size_t value) {
  for (uint8_t num_bits = sizeof(value) * CHAR_BIT; num_bits > __CHAR_BIT__;
       num_bits -= __CHAR_BIT__) {
    if (value >> num_bits) {
      return ceil((double)num_bits / CHAR_BIT);
    }
  }
  // Special-case: zero requires one, not zero bytes.
  return 1;
}

void InsertVariableInt(const size_t value,
                       const size_t pos,
                       std::vector<uint8_t>& der) {
  std::vector<uint8_t> variable_int;
  for (uint8_t shift = GetVariableIntLen(value); shift != 0; --shift) {
    variable_int.push_back((value >> ((shift - 1) * CHAR_BIT)) & 0xFF);
  }
  der.insert(der.begin() + pos, variable_int.begin(), variable_int.end());
}

void EncodeDefiniteLength(const size_t actual_len,
                          const size_t len_pos,
                          std::vector<uint8_t>& der) {
  InsertVariableInt(actual_len, len_pos, der);
  size_t len_num_bytes = GetVariableIntLen(actual_len);
  // X.690 (2015), 8.1.3.3: The long-form is used when the length is
  // larger than 127.
  // Note: |len_num_bytes| is not checked here, because it will equal
  // 1 for values [128..255], but those require the long-form length.
  if (actual_len > 127) {
    // See X.690 (2015) 8.1.3.5.
    // Long-form length is encoded as a byte with the high-bit set to indicate
    // the long-form, while the remaining bits indicate how many bytes are used
    // to encode the length.
    der.insert(der.begin() + len_pos, (0x80 | len_num_bytes));
  }
}

void EncodeIdentifier(const bool constructed,
                      const uint32_t tag_num,
                      std::vector<uint8_t>& der) {
  // The encoding, which is the 6th bit in the identifier, is 1 for constructed
  // (X.690 (2015), 8.1.2).
  uint8_t encoding = constructed ? 1 << 5 : 0;
  der.push_back((encoding | tag_num));
}