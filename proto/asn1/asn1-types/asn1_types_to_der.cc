#include "asn1_types_to_der.h"

namespace asn1_types {

std::vector<uint8_t> ASN1TypesToDER::EncodeBitString(
    const BitString& bit_string) {
  std::vector<uint8_t> der;
  EncodeTag(bit_string.encoding(), 0x03, 0, der);
  EncodeLength(bit_string.val().size(), 1, der);
  // Encode 0 to indicate that there are no unused bits.
  // This also acts as EOC if val is empty.
  der.push_back(0x00);
  der.insert(der.end(), bit_string.val().begin(), bit_string.val().end());
  return der;
}

std::vector<uint8_t> ASN1TypesToDER::EncodeInteger(const Integer& integer) {
  std::vector<uint8_t> der;
  // Integer has tag number 2 (X.208, Table 1) and is always primitive (X.690
  // (2015), 8.3.1).
  EncodeTag(false, 0x02, 0, der);
  EncodeLength(integer.val().size(), 1, der);
  der.insert(der.end(), integer.val().begin(), integer.val().end());
  return der;
}

std::vector<uint8_t> ASN1TypesToDER::EncodeUTCTime(const UTCTime& utc_time) {
  std::vector<uint8_t> der;
  EncodeTag(false, 0x17, 0, der);
  const google::protobuf::Descriptor* desc = utc_time.GetDescriptor();
  const google::protobuf::Reflection* ref = utc_time.GetReflection();
  for (int i = 0; i < 12; i++) {
    der.push_back(0x30 + ref->GetEnumValue(utc_time, desc->field(i)));
  }
  if (utc_time.zulu()) {
    der.push_back(0x5a);
    EncodeLength(13, 1, der);
  } else {
    EncodeLength(12, 1, der);
  }
  return der;
}

std::vector<uint8_t> ASN1TypesToDER::EncodeGeneralizedTime(
    const GeneralizedTime& generalized_time) {
  std::vector<uint8_t> der;
  // GeneralizedTime has tag number 24 (X.208, Table 1).
  EncodeTag(false, 0x18, 0, der);
  const google::protobuf::Descriptor* desc = generalized_time.GetDescriptor();
  const google::protobuf::Reflection* ref = generalized_time.GetReflection();
  for (int i = 0; i < 14; i++) {
    // GeneralizedTime is encoded like a string so add 0x30 to get ascii
    // character.
    der.push_back(0x30 + ref->GetEnumValue(generalized_time, desc->field(i)));
  }
  // The encoding shall terminate with "Z" (ITU-T X.680 | ISO/IEC 8824-1).
  if (generalized_time.zulu()) {
    der.push_back(0x5a);
    EncodeLength(15, 1, der);
  } else {
    EncodeLength(14, 1, der);
  }
  return der;
}

}  // namespace asn1_types