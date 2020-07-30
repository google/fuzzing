#ifndef COMMON_H_
#define COMMON_H_

#include <math.h>
#include <stdint.h>

#include <vector>

uint8_t GetVariableIntLen(const uint64_t value, const size_t base);

void InsertVariableInt(const size_t value,
                       const size_t pos,
                       std::vector<uint8_t>& der);

void EncodeTag(const bool constructed,
               const size_t tag_num,
               const size_t pos,
               std::vector<uint8_t>& der);

void EncodeLength(const size_t len,
                  const size_t len_pos,
                  const std::vector<uint8_t>& der);

#endif