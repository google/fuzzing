#ifndef COMMON_H_
#define COMMON_H_

#include <math.h>
#include <stdint.h>

#include <vector>

uint8_t GetVariableIntLen(const uint64_t value, const size_t base);

void InsertVariableInt(const size_t value,
                       const size_t pos,
                       std::vector<uint8_t>& der);

void EncodeTagAndLength(const size_t tag,
                        const size_t len,
                        const size_t pos,
                        std::vector<uint8_t>& der);

void EncodeDefiniteLength(const size_t actual_len,
                          const size_t len_pos,
                          const std::vector<uint8_t>& der);

#endif