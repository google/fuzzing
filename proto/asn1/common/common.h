#ifndef COMMON_H_
#define COMMON_H_

#include <math.h>
#include <stdint.h>

#include <vector>

// Returns the number of bytes needed to |base| encode |value| into a
// variable-length unsigned integer with no leading zeros.
uint8_t GetVariableIntLen(const uint64_t value, const size_t base);

// Converts |value| to a variable-length, big-endian representation and
// inserts the result into into |der| at |pos|.
void InsertVariableInt(const size_t value,
                       const size_t pos,
                       std::vector<uint8_t>& der);

// Encodes |tag| and |len| into |der| at |pos| according to X.690
// (2015), 8.1.2-8.1.5.
void EncodeTagAndLength(const size_t tag,
                        const size_t len,
                        const size_t pos,
                        std::vector<uint8_t>& der);

void SetTag(const size_t tag,
            const size_t pos_of_tag,
            std::vector<uint8_t>& der);

#endif