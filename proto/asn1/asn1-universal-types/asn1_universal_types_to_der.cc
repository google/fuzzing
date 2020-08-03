#include "asn1_universal_types_to_der.h"

namespace asn1_universal_types {

void Encode(const BitString& bit_string, std::vector<uint8_t>& der) {
  // BitString has tag number 3 and can be both constructed and primitive
  // (X.208, Table 1).
  // Add one to the length for the unused bits byte.
  EncodeTagAndLength((bit_string.encoding() << 5 | 0x03),
                     bit_string.val().size() + 1, der.size(), der);

  // Encode 0 to indicate that there are no unused bits.
  // This also acts as EOC if val is empty.
  der.push_back(0x00);
  der.insert(der.end(), bit_string.val().begin(), bit_string.val().end());
}

void Encode(const Integer& integer, std::vector<uint8_t>& der) {
  // Integer has tag number 2 (X.208, Table 1) and is always primitive (X.690
  // (2015), 8.3.1).
  EncodeTagAndLength(0x02, integer.val().size(), der.size(), der);

  der.insert(der.end(), integer.val().begin(), integer.val().end());
}

void Encode(const UTCTime& utc_time, std::vector<uint8_t>& der) {
  // Check for zulu and insert at the beginning to avoid memmove.
  uint8_t size = utc_time.zulu() ? 13 : 12;
  // UTCTime has tag number 23 (X.208, Table 1).
  EncodeTagAndLength(0x17, size, der.size(), der);

  const google::protobuf::Descriptor* desc = utc_time.GetDescriptor();
  const google::protobuf::Reflection* ref = utc_time.GetReflection();
  // |utc_time| has 12 time values (X.690 (2015), 11.8).
  for (int i = 0; i < 12; i++) {
    der.push_back(ref->GetEnumValue(utc_time, desc->field(i)));
  }

  CorrectTime(der.size() - 12, der);

  for(int i = der.size(); i > der.size() - 12; i--) {
    // UTCTime is encoded like a string so add 0x30 to get ascii character.
    der[i] += 0x30;
  }

  // The encoding shall terminate with "Z" (ITU-T X.680 | ISO/IEC 8824-1).
  if (utc_time.zulu()) {
    der.push_back(0x5a);
  }
}

void Encode(const GeneralizedTime& generalized_time,
            std::vector<uint8_t>& der) {
  // Check for zulu and insert at the beginning to avoid memmove.
  uint8_t size = generalized_time.zulu() ? 15 : 14;
  // GeneralizedTime has tag number 24 (X.208, Table 1).
  EncodeTagAndLength(0x18, size, der.size(), der);

  const google::protobuf::Descriptor* desc = generalized_time.GetDescriptor();
  const google::protobuf::Reflection* ref = generalized_time.GetReflection();
  // |generalized_tim| has 14 time values (X.690 (2015), 11.7).
  for (int i = 0; i < 14; i++) {
    der.push_back(ref->GetEnumValue(generalized_time, desc->field(i)));
  }

  CorrectTime(der.size() - 12, der);

  // GeneralizedTime is encoded like a string so add 0x30 to get ascii
  // character.
  for (int i = der.size(); i > der.size() - 14; i--) {
    der[i] += 0x30;
  }

  // The encoding shall terminate with "Z" (ITU-T X.680 | ISO/IEC 8824-1).
  if (generalized_time.zulu()) {
    der.push_back(0x5a);
  }
}

void CorrectTime(const size_t pos_of_year, std::vector<uint8_t>& der) {
  uint8_t month = (der[pos_of_year + 2] * 10) + der[pos_of_year + 3];
  if (month > 12 || month == 0) {
    der[pos_of_year + 3] = 1;
  }

  const uint8_t year = (der[pos_of_year] * 10) + der[pos_of_year + 1];
  const uint8_t month_fixed = (der[pos_of_year + 2] * 10) + der[pos_of_year + 3];
  const uint8_t day = (der[pos_of_year + 4] * 10) + der[pos_of_year + 5];
  std::vector<uint8_t> day_limits = {31, 28, 31, 30, 31, 30,
                                       31, 31, 30, 31, 30, 31};
  if (year % 4 == 0) {
    day_limits[1] = 29;
  }

  if(day > day_limits[month_fixed] || day == 0) {
    der[pos_of_year + 4] = 1;
  }
}

}  // namespace asn1_universal_types