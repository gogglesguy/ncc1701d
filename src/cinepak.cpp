#include "cinepak.h"

enum {
  intra_block_v4_12 = 0x2000,
  intra_block_v4_8 = 0x2200,
  intra_block_v1_12 = 0x2400,
  intra_block_v1_8 = 0x2600,
  vectors = 0x3000,
  intra_block_v1 = 0x3200,

  inter_block_v4_12 = 0x2100,
  inter_block_v4_8 = 0x2300,
  inter_block_v1_12 = 0x2500,
  inter_block_v1_8 = 0x2700,
  partial_vectors = 0x3100
  };



typedef std::uint32_t cinepak_color[4];


void decode_cinepak_frame(gap::stream & store, std::uint32_t* dst, std::uint32_t stride) {
  gap::stream::byteorder_scope scope(store,gap::endian::big);

  std::uint8_t flags;
  std::uint32_t size = 0;
  std::uint16_t width,height,nstrips;

  cinepak_color v1[256];
  cinepak_color v4[256];

  short v4n = 0;
  short v1n = 0;

  // Frame Header
  store >> flags;
  store.load_uint24(size);
  store >> width;
  store >> height;
  store >> nstrips;

  while(nstrips) {
    std::uint16_t stripid;
    std::uint16_t stripsize;
    std::uint16_t tx;
    std::uint16_t ty;
    std::uint16_t bx;
    std::uint16_t by;

    // Strip Header
    store >> stripid;
    store >> stripsize;
    store >> ty;
    store >> tx;
    store >> by;
    store >> bx;

    stripsize -= 12;

    // Decode a Strip
    while(stripsize) {
      std::uint16_t chunkid,chunksize;
      std::uint32_t chunkflags;
      std::uint32_t ncolors;

      store >> chunkid;
      store >> chunksize;

      stripsize -= chunksize;

      chunksize -= 4;

      switch(chunkid) {
        case intra_block_v4_12:
          {
            v4n = chunksize / 6;
            for (int n = 0; n < v4n; n++) {
              std::uint8_t luminance[4];
              std::int8_t u,v;

              store >> luminance;
              store >> u;
              store >> v;

              for (int c = 0; c < 4; c++) {
                std::uint32_t r = std::clamp(luminance[c] + (v*2), 0, 255);
                std::uint32_t g = std::clamp(luminance[c] - (u/2) - v, 0, 255);
                std::uint32_t b = std::clamp(luminance[c] + (u*2), 0, 255);
                v4[n][c] = (255 << 24) | (r<<16) | (g<<8) | b;
                }
              }
            break;
          }
        case vectors:
          {
            std::uint8_t r[4];

            int xx = tx;
            int yy = ty;

            while(chunksize > 4) {
              store >> chunkflags;
              chunksize -= 4;
              for (int b = 31; b >= 0 && chunksize; b--) {
                if (chunkflags & (1 << b)) {

                  if (chunksize < 4) throw -1;

                  store >> r;
                  chunksize -= 4;

                  //printf("block %d.%d\n",xx, yy);
                  std::uint32_t * out = dst + (stride * yy) + xx;

                  out[0] = v4[r[0]][0];
                  out[1] = v4[r[0]][1];
                  out[2] = v4[r[1]][0];
                  out[3] = v4[r[1]][1];

                  out += stride;

                  out[0] = v4[r[0]][2];
                  out[1] = v4[r[0]][3];
                  out[2] = v4[r[1]][2];
                  out[3] = v4[r[1]][3];

                  out += stride;

                  out[0] = v4[r[2]][0];
                  out[1] = v4[r[2]][1];
                  out[2] = v4[r[3]][0];
                  out[3] = v4[r[3]][1];

                  out += stride;

                  out[0] = v4[r[2]][2];
                  out[1] = v4[r[2]][3];
                  out[2] = v4[r[3]][2];
                  out[3] = v4[r[3]][3];
                  }
                else {
                  printf("not implemented v1 vector %d %d %d\n",xx, yy, chunksize);
                  throw -1;
                  }

                xx += 4;
                if (xx >= bx-1) {
                  xx = tx;
                  yy += 4;
                  if (yy >= by-1) break;
                  }
                }
              }
            store += chunksize; // left over bytes
            break;
          }
        }
      }
    nstrips --;
    }
  }
