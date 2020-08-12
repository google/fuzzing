// Copyright 2020 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "asn1_universal_types_to_der.h"

#include <algorithm>

#include <google/protobuf/util/time_util.h>
#include "common.h"

namespace asn1_universal_types {

void Encode(const Boolean& boolean, std::vector<uint8_t>& der) {
  der.push_back(kAsn1Boolean);
  // The contents octets shall consist of a single octet (X.690 (2015), 8.2.1).
  // Therefore, length is always 1.
  der.push_back(0x01);

  if (boolean.val()) {
    // If the boolean value is TRUE, the octet shall have any non-zero value
    // (X.690 (2015), 8.2.2).
    der.push_back(0xFF);
  } else {
    // If the boolean value is FALSE, the octet shall be zero (X.690
    // (2015), 8.2.2).
    der.push_back(0x00);
  }
}

void Encode(const Integer& integer, std::vector<uint8_t>& der) {
  EncodeTagAndLength(kAsn1Integer,
                     std::min<size_t>(0x01u, integer.val().size()), der.size(),
                     der);

  if (!integer.val().empty()) {
    der.insert(der.end(), integer.val().begin(), integer.val().end());
  } else {
    // Cannot have an empty integer, so use the value 0.
    der.push_back(0x00);
  }
}

void Encode(const BitString& bit_string, std::vector<uint8_t>& der) {
  EncodeTagAndLength(kAsn1Bitstring, bit_string.val().size() + 1, der.size(),
                     der);

  if (!bit_string.val().empty()) {
    der.push_back(bit_string.unused_bits());
    der.insert(der.end(), bit_string.val().begin(), bit_string.val().end());
  } else {
    // If the bitstring is empty, there shall be no subsequent octets,
    // and the initial octet shall be zero (X.690 (2015), 8.6.2.3).
    der.push_back(0x00);
  }
}

void Encode(const ObjectIdentifier& object_identifier,
            std::vector<uint8_t>& der) {
  // Save the current size in |tag_len_pos| to place tag and length
  // after the value is encoded.
  const size_t tag_len_pos = der.size();

  uint8_t root = object_identifier.root();
  uint8_t small_identifier = object_identifier.small_identifier();
  auto subidentifier = object_identifier.subidentifier();

  // (X.690 (2015) 8.19.4): Only 39 subsequent values from nodes reached by X =
  // 0 and X = 1. Therefore, use |small_identifier| for |root| 0 or 1, and when
  // |root| is 2, use first integer in |subidentifier| to obtain
  // potentially higher values.
  size_t identifier = (root == 2 && !subidentifier.empty())
                          ? (root * 40) + subidentifier[0]
                          : (root * 40) + small_identifier;
  der.push_back(identifier);

  if (!subidentifier.empty()) {
    for (uint32_t value : subidentifier) {
      // The subidentifier is base 128 encoded (X.690 (2015), 8.19.2).
      InsertVariableIntBase128(value, der.size(), der);
    }
  } else {
    // Cannot have an empty integer, so use the value 0.
    der.push_back(0x00);
  }

  EncodeTagAndLength(kAsn1ObjectIdentifier, der.size() - tag_len_pos,
                     tag_len_pos, der);
}

void Encode(const UTCTime& utc_time, std::vector<uint8_t>& der) {
  // Save the current size in |tag_len_pos| to place tag and length
  // after the value is encoded.
  const size_t tag_len_pos = der.size();

  EncodeTimestamp(utc_time.time_stamp(), false, der);

  // Check if encoding was successful.
  if (der.size() != tag_len_pos) {
    EncodeTagAndLength(kAsn1UTCTime, der.size() - tag_len_pos, tag_len_pos,
                       der);
  }
}

void Encode(const GeneralizedTime& generalized_time,
            std::vector<uint8_t>& der) {
  // Save the current size in |tag_len_pos| to place tag and length
  // after the value is encoded.
  const size_t tag_len_pos = der.size();

  EncodeTimestamp(generalized_time.time_stamp(), true, der);

  // Check if encoding was successful.
  if (der.size() != tag_len_pos) {
    EncodeTagAndLength(kAsn1Generalizedtime, der.size() - tag_len_pos,
                       tag_len_pos, der);
  }
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
  // Partitioning the year ensure always valid encodings, i.e. if
  // 1850 is being encoded as a UTCTime, it will be encoded as
  // '50' for the year, rather than an error.
  time_str += use_two_digit_year ? iso_date.substr(2, 2)
                                 : iso_date.substr(0, 4);  // Year
  time_str += iso_date.substr(5, 2);                       // Month
  time_str += iso_date.substr(8, 2);                       // Day
  time_str += iso_date.substr(11, 2);                      // Hour
  time_str += iso_date.substr(14, 2);                      // Minute
  time_str += iso_date.substr(17, 2);                      // Seconds
  // See X.690 (2015), 11.7.1 & 11.8.1: Encoding terminates with "Z".
  time_str += "Z";

  der.reserve(der.size() + time_str.size());
  std::transform(time_str.begin(), time_str.end(), std::back_inserter(der),
                 [](char c) -> size_t { return c; });
}

}  // namespace asn1_universal_types