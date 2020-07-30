#include "common.h"

uint8_t GetVariableIntLen(const uint64_t value, const size_t base) {
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

void InsertVariableInt(const size_t value,
                       const size_t pos,
                       std::vector<uint8_t>& der) {
  std::vector<uint8_t> variable_int;
  for (uint8_t shift = GetVariableIntLen(value, 256); shift != 0; --shift) {
    variable_int.push_back((value >> ((shift - 1) * CHAR_BIT)) & 0xFF);
  }
  der.insert(der.begin() + pos, variable_int.begin(), variable_int.end());
}

void EncodeTagAndLength(const size_t tag,
                        const size_t len,
                        const size_t pos,
                        std::vector<uint8_t>& der) {
  InsertVariableInt(len, pos, der);
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
  der.insert(der.begin() + pos, tag);
}