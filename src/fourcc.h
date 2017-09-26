#ifndef FOURCC_H
#define FOURCC_H

#include <cstdint>
#include "stream.h"

struct fourcc {
  std::uint32_t value = 0;

  fourcc() = default;

  constexpr fourcc(char const (&t)[5]) : value(t[0] << 24 |
                                               t[1] << 16 |
                                               t[2] << 8 |
                                               t[3]) {}
  const char* text() const {
    static char txt[5];
    txt[0] = (value >> 24) & 0xff;
    txt[1] = (value >> 16) & 0xff;
    txt[2] = (value >> 8) & 0xff;
    txt[3] = (value) & 0xff;
    txt[4] = '\0';
    return txt;
    }

  constexpr operator std::uint32_t() const { return value; }
  };


extern gap::stream & operator>>(gap::stream& store, fourcc & t);


#endif
