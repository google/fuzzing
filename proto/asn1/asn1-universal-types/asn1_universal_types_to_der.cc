#include "asn1_universal_types_to_der.h"
#include <google/protobuf/util/time_util.h>
#include <time.h>

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
  const size_t size_before = der.size();

  const auto time_stamp = utc_time.time_stamp();
  time_t rawtime =
      google::protobuf::util::TimeUtil::TimestampToTimeT(time_stamp);
  struct tm* ptm;
  ptm = gmtime(&rawtime);

  const uint16_t years_since_1900 = ptm->tm_year;
  der.push_back(0x30 + 0x01 + (years_since_1900 / 1000));
  der.push_back(0x30 + 0x09 + (years_since_1900 / 100));
  der.push_back(0x30 + (years_since_1900 / 10));
  der.push_back(0x30 + (years_since_1900 % 10));

  const uint8_t months_since_jan = ptm->tm_mon;
  der.push_back(0x31 + (months_since_jan / 10));
  der.push_back(0x31 + (months_since_jan % 10));

  const uint8_t day_of_month = ptm->tm_mday;
  der.push_back(0x30 + (day_of_month / 10));
  der.push_back(0x30 + (day_of_month % 10));

  const uint8_t hours_since_midnight = ptm->tm_hour;
  der.push_back(0x30 + (hours_since_midnight / 10));
  der.push_back(0x30 + (hours_since_midnight % 10));

  const uint8_t minutes_after_hour = ptm->tm_min;
  der.push_back(0x30 + (minutes_after_hour / 10));
  der.push_back(0x30 + (minutes_after_hour % 10));

  // The encoding shall terminate with "Z" (X.690 (2015), 11.7.1 | ISO/IEC
  // 8824-1).
  der.push_back(0x5a);

  // UTCTime has tag number 23 (X.208, Table 1).
  EncodeTagAndLength(0x17, size_before - der.size(), der.size(), der);
}

void Encode(const GeneralizedTime& generalized_time,
            std::vector<uint8_t>& der) {
  const size_t size_before = der.size();

  const auto time_stamp = generalized_time.time_stamp();
  time_t rawtime =
      google::protobuf::util::TimeUtil::TimestampToTimeT(time_stamp);
  struct tm* ptm;
  ptm = gmtime(&rawtime);

  const uint16_t years_since_1900 = ptm->tm_year;
  der.push_back(0x30 + 0x01 + (years_since_1900 / 1000));
  der.push_back(0x30 + 0x09 + (years_since_1900 / 100));
  der.push_back(0x30 + (years_since_1900 / 10));
  der.push_back(0x30 + (years_since_1900 % 10));

  const uint8_t months_since_jan = ptm->tm_mon;
  der.push_back(0x31 + (months_since_jan / 10));
  der.push_back(0x31 + (months_since_jan % 10));

  const uint8_t day_of_month = ptm->tm_mday;
  der.push_back(0x30 + (day_of_month / 10));
  der.push_back(0x30 + (day_of_month % 10));

  const uint8_t hours_since_midnight = ptm->tm_hour;
  der.push_back(0x30 + (hours_since_midnight / 10));
  der.push_back(0x30 + (hours_since_midnight % 10));

  const uint8_t minutes_after_hour = ptm->tm_min;
  der.push_back(0x30 + (minutes_after_hour / 10));
  der.push_back(0x30 + (minutes_after_hour % 10));

  // The encoding shall terminate with "Z" (X.690 (2015), 11.7.1 | ISO/IEC
  // 8824-1).
  der.push_back(0x5a);

  // GeneralizedTime has tag number 24 (X.208, Table 1).
  EncodeTagAndLength(0x18, size_before - der.size(), der.size(), der);
}

}  // namespace asn1_universal_types