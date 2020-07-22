#include "asn1_proto_to_der.h"

namespace asn1_pdu {

// The maximum level of recursion allowed. Values greater than this will just
// fail.
static constexpr size_t kRecursionLimit = 200;

// Signal that the recursion limit has been exceeded by a
// message or value and to abort further processing.
static bool kErrRecursionLimitReached = false;

uint8_t ASN1ProtoToDER::GetVariableIntLen(const uint64_t value,
                                          const size_t base) {
  uint8_t base_bits = log2(base);
  for (uint8_t num_bits = (sizeof(value) - 1) * 8; num_bits >= base_bits;
       num_bits -= base_bits) {
    if ((value >> num_bits)) {
      return ceil((double)num_bits / base_bits) + 1;
    }
  }
  // Special-case: zero requires one, not zero bytes.
  return 1;
}

void ASN1ProtoToDER::InsertVariableInt(const size_t value, const size_t pos) {
  std::vector<uint8_t> variable_int;
  for (uint8_t shift = GetVariableIntLen(value, 256); shift != 0; --shift) {
    variable_int.push_back((value >> ((shift - 1) * 8)) & 0xFF);
  }
  encoder_.insert(encoder_.begin() + pos, variable_int.begin(),
                  variable_int.end());
}

void ASN1ProtoToDER::EncodeOverrideLength(const std::string raw_len,
                                          const size_t len_pos) {
  encoder_.insert(encoder_.begin() + len_pos, raw_len.begin(), raw_len.end());
}

void ASN1ProtoToDER::EncodeIndefiniteLength(const size_t len_pos) {
  encoder_.insert(encoder_.begin() + len_pos, 0x80);
  // The PDU's value is from |len_pos| to the end of |encoder_|, so just add an
  // EOC marker to the end.
  encoder_.push_back(0x00);
  encoder_.push_back(0x00);
}

void ASN1ProtoToDER::EncodeDefiniteLength(const size_t actual_len,
                                          const size_t len_pos) {
  InsertVariableInt(actual_len, len_pos);
  size_t len_num_bytes = GetVariableIntLen(actual_len, 256);
  // X.690 (2015), 8.1.3.3: The long-form is used when the length is
  // larger than 127.
  // Note: |len_num_bytes| is not checked here, because it will equal
  // 1 for values [128..255], but those require the long-form length.
  if (actual_len > 127) {
    // See X.690 (2015) 8.1.3.5.
    // Long-form length is encoded as a byte with the high-bit set to indicate
    // the long-form, while the remaining bits indicate how many bytes are used
    // to encode the length.
    encoder_.insert(encoder_.begin() + len_pos, (0x80 | len_num_bytes));
  }
}

void ASN1ProtoToDER::EncodeLength(const Length& len,
                                  const size_t actual_len,
                                  const size_t len_pos) {
  if (len.has_length_override()) {
    EncodeOverrideLength(len.length_override(), len_pos);
  } else if (len.has_indefinite_form() && len.indefinite_form()) {
    EncodeIndefiniteLength(len_pos);
  } else {
    EncodeDefiniteLength(actual_len, len_pos);
  }
}

void ASN1ProtoToDER::EncodeValue(const Value& val) {
  for (const auto& val_ele : val.val_array()) {
    if (!kErrRecursionLimitReached) {
      if (val_ele.has_pdu()) {
        EncodePDU(val_ele.pdu());
      } else {
        encoder_.insert(encoder_.end(), val_ele.val_bits().begin(),
                        val_ele.val_bits().end());
      }
    }
  }
}

void ASN1ProtoToDER::EncodeHighTagNumberForm(const uint8_t id_class,
                                             const uint8_t encoding,
                                             const uint32_t tag_num) {
  // The high-tag-number form base 128 encodes |tag_num| (X.690 (2015), 8.1.2).
  uint8_t num_bytes = GetVariableIntLen(tag_num, 128);
  // High-tag-number form requires the lower 5 bits of the identifier to be set
  // to 1 (X.690 (2015), 8.1.2.4.1).
  uint64_t id_parsed = (id_class | encoding | 0x1F);
  id_parsed <<= 8;
  for (uint8_t i = num_bytes - 1; i != 0; --i) {
    // If it's not the last byte, the high bit is set to 1 (X.690
    // (2015), 8.1.2.4.2).
    id_parsed |= ((0x01 << 7) | ((tag_num >> (i * 7)) & 0x7F));
    id_parsed <<= 8;
  }
  id_parsed |= (tag_num & 0x7F);
  InsertVariableInt(id_parsed, encoder_.size());
}

void ASN1ProtoToDER::EncodeIdentifier(const Identifier& id) {
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
    encoder_.push_back(static_cast<uint8_t>(id_class | encoding | tag_num));
  }
}

void ASN1ProtoToDER::EncodePDU(const PDU& pdu) {
  ++depth_;
  // Artifically limit the stack depth to avoid stack overflow.
  if (depth_ > kRecursionLimit) {
    kErrRecursionLimitReached = true;
    return;
  }
  size_t size_before_insertion = encoder_.size();
  EncodeIdentifier(pdu.id());
  size_t id_size = encoder_.size() - size_before_insertion;
  EncodeValue(pdu.val());
  size_t value_size = encoder_.size() - size_before_insertion - id_size;
  EncodeLength(pdu.len(), value_size, size_before_insertion + id_size);
  --depth_;
}

std::vector<uint8_t> ASN1ProtoToDER::ProtoToDER(const PDU& pdu) {
  // The same instantiaton of ANS1ProtoToDER can encode multiple PDU's.
  // The following three instructions are used to reset the values
  // before each PDU is encoded.
  encoder_.clear();
  depth_ = 0;
  kErrRecursionLimitReached = false;
  EncodePDU(pdu);
  if (kErrRecursionLimitReached) {
    encoder_.clear();
  }
  return encoder_;
}

}  // namespace asn1_pdu