#include "asn1_types_to_der.h"

namespace asn1_types {

std::vector<uint8_t> ASN1TypesToDER::EncodeBitString(
    const BitString& bit_string) {
  std::vector<uint8_t> der;
  // BitString has tag number 3 and can be both constructed and primitive
  // (X.208, Table 1).
  // Add one to the length for the unused bits byte.
  EncodeTagAndLength((bit_string.encoding() << 5 | 0x03),
                     bit_string.val().size() + 1, 0, der);

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
  EncodeTagAndLength(0x02, integer.val().size(), 0, der);

  der.insert(der.end(), integer.val().begin(), integer.val().end());
  return der;
}

std::vector<uint8_t> ASN1TypesToDER::EncodeUTCTime(const UTCTime& utc_time) {
  std::vector<uint8_t> der;
  const google::protobuf::Descriptor* desc = utc_time.GetDescriptor();
  const google::protobuf::Reflection* ref = utc_time.GetReflection();
  for (int i = 0; i < 12; i++) {
    // UTCTime is encoded like a string so add 0x30 to get ascii character.
    der.push_back(0x30 + ref->GetEnumValue(utc_time, desc->field(i)));
  }

  // The encoding shall terminate with "Z" (ITU-T X.680 | ISO/IEC 8824-1).
  // UTCTime has tag number 23 (X.208, Table 1).
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
    // GeneralizedTime is encoded like a string so add 0x30 to get ascii
    // character.
    der.push_back(0x30 + ref->GetEnumValue(generalized_time, desc->field(i)));
  }
  
  // The encoding shall terminate with "Z" (ITU-T X.680 | ISO/IEC 8824-1).
  // GeneralizedTime has tag number 24 (X.208, Table 1).
  if (generalized_time.zulu()) {
    der.push_back(0x5a);
    EncodeTagAndLength(0x18, 15, 0, der);
  } else {
    EncodeTagAndLength(0x18, 14, 0, der);
  }
  return der;
}

}  // namespace asn1_types