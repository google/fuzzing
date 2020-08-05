#ifndef PROTO_ASN1_COMMON_H_
#define PROTO_ASN1_COMMON_H_

#include <math.h>
#include <stdint.h>

#include <vector>

constexpr uint8_t kAsn1Constructed = 0x20u;
constexpr uint8_t kAsn1Universal = 0u;
constexpr uint8_t kAsn1Application = 0x40u;
constexpr uint8_t kAsn1ContextSpecific = 0x80u;
constexpr uint8_t kAsn1Private = 0xc0u;

constexpr uint8_t kAsn1Boolean = kAsn1Universal | 0x1u;
constexpr uint8_t kAsn1Integer = kAsn1Universal | 0x2u;
constexpr uint8_t kAsn1Bitstring = kAsn1Universal | 0x03u;
constexpr uint8_t kAsn1UTCTime = kAsn1Universal | 0x17u;
constexpr uint8_t kAsn1Generalizedtime = kAsn1Universal | 0x18u;

constexpr uint8_t kAsn1Sequence = kAsn1Universal | kAsn1Constructed | 0x10u;

// Returns the number of bytes needed to |base| encode |value| into a
// variable-length unsigned integer with no leading zeros.
uint8_t GetVariableIntLen(uint64_t value, size_t base);

// Converts |value| to a variable-length, big-endian representation and
// inserts the result into into |der| at |pos|.
void InsertVariableInt(uint64_t value,
                       size_t pos,
                       std::vector<uint8_t>& der);

// Encodes |tag| and |len| into |der| at |pos| according to X.690
// (2015), 8.1.2-8.1.5.
void EncodeTagAndLength(uint8_t tag,
                        size_t len,
                        size_t pos,
                        std::vector<uint8_t>& der);

void ReplaceTag(uint8_t tag_byte,
            size_t pos_of_tag,
            std::vector<uint8_t>& der);

#endif