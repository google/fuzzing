#include "asn1_types_to_der.h"

namespace asn1_types {

std::vector<uint8_t> ASN1TypesToDER::EncodeBitString(
    const BitString& bit_string) {
  std::vector<uint8_t> der;
  EncodeTagAndLength((bit_string.encoding() << 5 | 0x03),
                     bit_string.val().size(), 0, der);
  der.push_back(0x00);
  der.insert(der.end(), bit_string.val().begin(), bit_string.val().end());
  return der;
}

std::vector<uint8_t> ASN1TypesToDER::EncodeInteger(const Integer& integer) {
  std::vector<uint8_t> der;
  EncodeTagAndLength(0x02, integer.val().size(), 0, der);
  der.insert(der.end(), integer.val().begin(), integer.val().end());
  return der;
}

std::vector<uint8_t> ASN1TypesToDER::EncodeUTCTime(const UTCTime& utc_time) {
  std::vector<uint8_t> der;
  const google::protobuf::Descriptor* desc = utc_time.GetDescriptor();
  const google::protobuf::Reflection* ref = utc_time.GetReflection();
  for (int i = 0; i < 12; i++) {
    der.push_back(0x30 + ref->GetEnumValue(utc_time, desc->field(i)));
  }
  if (utc_time.zulu()) {
    der.push_back(0x5a);
    EncodeTagAndLength(0x17, 13, 0, der);
  } else {
    EncodeTagAndLength(0x17, 12, 0, der);
  }
  return der;
}

std::vector<uint8_t> ASN1TypesToDER::EncodeGeneralizedTime(
    const GeneralizedTime& generalized_time) {
  std::vector<uint8_t> der;
  const google::protobuf::Descriptor* desc = generalized_time.GetDescriptor();
  const google::protobuf::Reflection* ref = generalized_time.GetReflection();
  for (int i = 0; i < 14; i++) {
    der.push_back(0x30 + ref->GetEnumValue(generalized_time, desc->field(i)));
  }
  // The encoding shall terminate with "Z" (ITU-T X.680 | ISO/IEC 8824-1).
  if (generalized_time.zulu()) {
    der.push_back(0x5a);
    EncodeTagAndLength(0x18, 15, 0, der);
  } else {
    EncodeTagAndLength(0x18, 14, 0, der);
  }
  return der;
}

}  // namespace asn1_types