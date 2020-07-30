#ifndef COMMON_H_
#define COMMON_H_

#include <math.h>
#include <stdint.h>

#include <vector>

uint8_t GetVariableIntLen(size_t value);

void InsertVariableInt(const size_t value,
                       const size_t pos,
                       std::vector<uint8_t>& der);

void EncodeDefiniteLength(const size_t actual_len,
                          const size_t len_pos,
                          std::vector<uint8_t>& der);

void EncodeIdentifier(const bool constructed,
                      const uint32_t tag_num,
                      std::vector<uint8_t>& der);
#endif