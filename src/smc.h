#ifndef SMC_H
#define SMC_H

#include <cstdint>
#include "stream.h"

extern void decode_smc_frame(gap::stream & store, std::uint8_t * prev, std::uint8_t * dst, int width, int height, int stride);

#endif
