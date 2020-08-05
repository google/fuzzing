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

// Integer is UNIVERSAL 2 (X.680 (2015), 8.6, Table 1) and is always primitive
// (X.690 (2015), 8.3.1).
constexpr uint8_t kAsn1Integer = kAsn1Universal | 0x2u;
// Bitstring is UNIVERSAL 3 (X.680 (2015), 8.6, Table 1) and is always
// primitive in DER (X.690 (2015), 10.2).
constexpr uint8_t kAsn1Bitstring = kAsn1Universal | 0x03u;
// UTCTime has tag number 23 (X.680 (2015), 8.6, Table 1) and is always
// primitive in DER encoding (A Layman's Guide to a Subset of ASN.1, BER, and
// DER, 5.17).
constexpr uint8_t kAsn1UTCTime = kAsn1Universal | 0x17u;
// GeneralizedTime has tag number 24 (X.680 (2015), 8.6, Table 1) and is always
// primitive in DER encoding (A Layman's Guide to a Subset of ASN.1, BER, and
// DER, 5.17).
constexpr uint8_t kAsn1Generalizedtime = kAsn1Universal | 0x18u;

constexpr uint8_t kAsn1Sequence = kAsn1Universal | kAsn1Constructed | 0x10u;

// Returns the number of bytes needed to |base| encode |value| into a
// variable-length unsigned integer with no leading zeros.
uint8_t GetVariableIntLen(uint64_t value, size_t base);

// Converts |value| to a variable-length, big-endian representation and
// inserts the result into into |der| at |pos|.
void InsertVariableInt(uint64_t value, size_t pos, std::vector<uint8_t>& der);

// Encodes |tag_byte| and |len| into |der| at |pos| according to X.690
// (2015), 8.1.2-8.1.5.
void EncodeTagAndLength(uint8_t tag_byte,
                        size_t len,
                        size_t pos,
                        std::vector<uint8_t>& der);

// Updates the tag in |der| to a single byte |tag_byte| assuming it already has
// been written by EncodetagAndLength or EncodeIdentifier at |pos_of_tag|.
void ReplaceTag(uint8_t tag_byte, size_t pos_of_tag, std::vector<uint8_t>& der);

#endif