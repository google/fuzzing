#ifndef ASN1_PROTO_TO_DER_H_
#define ASN1_PROTO_TO_DER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "asn1_pdu.pb.h"

namespace asn1_pdu {

class ASN1ProtoToDER {
 public:
  // Encodes |pdu| to DER, returning the encoded bytes of the PDU in
  // |encoder_|.
  std::vector<uint8_t> ProtoToDER(const PDU& pdu);

 private:
  std::vector<uint8_t> encoder_;

  // Enocdes |pdu| to DER, returning the number of bytes used to encode |pdu|.
  void EncodePDU(const PDU& pdu);

  // Encodes |id| to DER according to X.690 (2015), 8.1.2.
  // Returns number of bytes used to encode |id|.
  void EncodeIdentifier(const Identifier& id);

  // Concatinates |id_class|, |encoding|, and |tag| according to DER
  // high-tag-number form rules (X.690 (2015), 8.1.2.4).
  void EncodeHighTagNumberForm(const uint8_t id_class,
                               const uint8_t encoding,
                               const uint32_t tag);

  // Encodes the length to DER.
  // |len| can be used to affect the encoding, in order to produce
  // invalid lengths. |actual_len| is the correct length of the PDU,
  // and is used when |len| is not. |len_pos| contains the offset in |encoder_|
  // where the length should be encoded.
  // To correctly call this, the tag must already be encoded immediately prior
  // to |len_pos|, and the remainder of |encoder_| represents the encoded value.
  void EncodeLength(const Length& len, size_t actual_len, size_t len_pos);

  // Writes |raw_len| to |encoder_| at |len_pos|.
  void EncodeOverrideLength(const std::string& raw_len, const size_t len_pos);

  // Encodes the indefinite-length indicator (X.690 (2015), 8.1.3.6) at
  // |len_pos|, and appends an End-of-Contents (EOC) marker at the end of
  // |encoder_|.
  void EncodeIndefiniteLength(const size_t len_pos);

  // Encodes the length in |actual_len| using the definite-form length (X.690
  // (2015), 8.1.3-8.1.5 & 10.1) into |encoder_| at |len_pos|.
  void EncodeDefiniteLength(const size_t actual_len, const size_t len_pos);

  // Extracts bytes from |val| and inserts them into |enocder_|.
  void EncodeValue(const Value& val);

  // Converts |value| to a variable-length, big-endian representation and
  // inserts the result into into |encoder_| at |pos|.
  void InsertVariableInt(const size_t value, const size_t pos);

  // Returns the number of bytes needed to |base| encode |value| into a
  // variable-length unsigned integer with no leading zeros.
  uint8_t GetVariableIntLen(const uint64_t value, const size_t base);

  // Tracks recursion depth to avoid stack exhaustion.
  size_t depth_;
};

}  // namespace asn1_pdu

#endif
