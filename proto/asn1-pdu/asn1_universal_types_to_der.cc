#include "asn1_universal_types_to_der.h"

#include <google/protobuf/util/time_util.h>

namespace asn1_universal_types {

void Encode(const Integer& integer, std::vector<uint8_t>& der) {
  // Save the current size in |tag_len_pos| to place tag and length
  // after the value is encoded.
  const size_t tag_len_pos = der.size();

  if (!integer.val().empty()) {
    der.insert(der.end(), integer.val().begin(), integer.val().end());
  } else {
    // Cannot have an empty integer, so use the value 0.
    der.push_back(0x00);
  }

  EncodeTagAndLength(kAsn1Integer, der.size() - tag_len_pos, tag_len_pos, der);
}

void Encode(const BitString& bit_string, std::vector<uint8_t>& der) {
  // Save the current size in |tag_len_pos| to place tag and length
  // after the value is encoded.
  const size_t tag_len_pos = der.size();

  if (!bit_string.val().empty()) {
    der.push_back(bit_string.unused_bits());
    der.insert(der.end(), bit_string.val().begin(), bit_string.val().end());
  } else {
    // If the bitstring is empty, there shall be no subsequent octets,
    // and the initial octet shall be zero (X.690 (2015), 8.6.2.3).
    der.push_back(0x00);
  }

  EncodeTagAndLength(kAsn1Bitstring, der.size() - tag_len_pos, tag_len_pos,
                     der);
}

void Encode(const UTCTime& utc_time, std::vector<uint8_t>& der) {
  // Save the current size in |tag_len_pos| to place tag and length
  // after the value is encoded.
  const size_t tag_len_pos = der.size();

  EncodeTimestamp(utc_time.time_stamp(), 13, der);

  // Check if encoding was unsuccessful.
  if (der.size() == tag_len_pos) {
    return;
  }

  EncodeTagAndLength(kAsn1UTCTime, der.size() - tag_len_pos, tag_len_pos, der);
}

void Encode(const GeneralizedTime& generalized_time,
            std::vector<uint8_t>& der) {
  // Save the current size in |tag_len_pos| to place tag and length
  // after the value is encoded.
  const size_t tag_len_pos = der.size();

  EncodeTimestamp(generalized_time.time_stamp(), 15, der);

  // Check if encoding was unsuccessful.
  if (der.size() == tag_len_pos) {
    return;
  }

  EncodeTagAndLength(kAsn1Generalizedtime, der.size() - tag_len_pos,
                     tag_len_pos, der);
}

void EncodeTimestamp(const google::protobuf::Timestamp& timestamp,
                     bool use_two_digit_year,
                     std::vector<uint8_t>& der) {
  std::string iso_date = google::protobuf::util::TimeUtil::ToString(timestamp);
  if (iso_date.size() < 25) {
    return;
  }

  std::string time_str;
  // See X.690 (2015), 11.7.5: GeneralizedTime also includes the thousands digit
  // and hundreds digit of the year to support dates after 2050 by representing
  // the year with four digits.
  // See X.690 (2015), 11.8.3: UTCTime represents dates between 1950 and 2050,
  // so need only use tens and ones digit of the year.
  // Paritioning the year ensure always valid encodings, i.e. need not
  // check if the year spans a certain range.
  time_str += use_two_digit_year ? iso_date.substr(2, 2)
                                 : iso_date.substr(0, 4);  // Year
  time_str += iso_date.substr(5, 2);                       // Month
  time_str += iso_date.substr(8, 2);                       // Day
  time_str += iso_date.substr(11, 2);                      // Hour
  time_str += iso_date.substr(14, 2);                      // Minute
  time_str += iso_date.substr(17, 2);                      // Seconds
  // See X.690 (2015), 11.7.1 & 11.8.1: Encoding terminates with "Z".
  time_str += "Z";

  for (const char c : time_str) {
    der.push_back(static_cast<uint8_t>(c));
  }
}

}  // namespace asn1_universal_types