#ifndef CINEPACK_H
#define CINEPACK_H

#include <cstdint>
#include "stream.h"

extern void decode_cinepak_frame(gap::stream & store, std::uint32_t * buffer, std::uint32_t stride);

#endif
