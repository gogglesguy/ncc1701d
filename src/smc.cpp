#include "smc.h"

typedef std::uint8_t smc_octet_t[8];
typedef std::uint8_t smc_pair_t[2];
typedef std::uint8_t smc_quad_t[4];

void decode_smc_frame(gap::stream & store, std::uint8_t * prev, std::uint8_t * dest, int ww, int hh, int stride) {

  const int bytes_per_pixel = 1;

  // palette
  std::uint8_t pal[256];
  for (int c=0;c<256;c++) pal[c] = c;

  std::uint32_t unknown;
  std::uint8_t opcode;
  std::uint8_t index;
  std::uint8_t value;
  std::uint8_t octets[8];
  std::uint8_t octets_flags[6];
  std::uint8_t hextet[16];
  std::uint8_t quads[4];
  std::uint8_t quads_flags[4];
  std::uint8_t pairs[2];
  std::uint8_t pairs_flags[2];

  smc_octet_t c8[256] = {};
  int c8_index=0;

  smc_pair_t c2[256] = {};
  int c2_index=0;

  smc_quad_t c4[256] = {};
  int c4_index=0;

  store >> unknown;

  int xx = 0;
  int yy = 0;
  int count;

  enum {
    op_skip      = 0x0,
    op_skip_n    = 0x1,
    op_repeat1   = 0x2,
    op_repeat1_n = 0x3,
    op_repeat2   = 0x4,
    op_repeat2_n = 0x5,
    op_color1    = 0x6,
    op_color1_n  = 0x7,
    op_color2    = 0x8,
    op_color2_n  = 0x9,
    op_color4    = 0xa,
    op_color4_n  = 0xb,
    op_color8    = 0xc,
    op_color8_n  = 0xd,
    op_unknown   = 0xf
    };

  while(1) {
    store >> opcode;
    count = 1 + (opcode&0xf);
    opcode >>= 4;
    switch(opcode) {
      case op_skip:
      case op_skip_n:
        {
          if (opcode == op_skip_n) {
            store >> value;
            count = 1 + value;
            }

          if (prev == nullptr) {
            printf("need previous frame\n");
            throw -1;
            }

          count *= 4;

          for (int bb = 0; bb < count;) {
            int n = std::min(count, (ww - xx));
            memcpy(dest + (yy * stride) + xx, prev + (yy * stride) + xx, bytes_per_pixel * n);
            memcpy(dest + ((1 + yy) * stride) + xx, prev + ((1 + yy) * stride) + xx, bytes_per_pixel * n);
            memcpy(dest + ((2 + yy) * stride) + xx, prev + ((2 + yy) * stride) + xx, bytes_per_pixel * n);
            memcpy(dest + ((3 + yy) * stride) + xx, prev + ((3 + yy) * stride) + xx, bytes_per_pixel * n);
            bb += n;
            xx += n;
            if (xx >= ww) {
              xx = 0;
              yy += 4;
              if (yy >= hh) goto done;
              }
            }
          break;
        }

      case op_repeat1:
      case op_repeat1_n:
        {
          if (opcode == op_repeat1_n) {
            store >> value;
            count = 1 + value;
            }

          int px = xx - 4;
          int py = yy;
          if (px < 0) {
            px = ww - 4;
            py = yy - 4;
            }

          count *= 4;

          while(count > 0) {

            int n = std::min((xx + count), ww);

            for (int x = xx; x < n; x += 4)
              memcpy(dest + (yy * stride) + x, dest + (py * stride) + px, 4 * bytes_per_pixel);
            for (int x = xx; x < n; x += 4)
              memcpy(dest + ((1 + yy) * stride) + x, dest + ((1 + py) * stride) + px, 4 * bytes_per_pixel);
            for (int x = xx; x < n; x += 4)
              memcpy(dest + ((2 + yy) * stride) + x, dest + ((2 + py) * stride) + px, 4 * bytes_per_pixel);
            for (int x = xx; x < n; x += 4)
              memcpy(dest + ((3 + yy) * stride) + x, dest + ((3 + py) * stride) + px, 4 * bytes_per_pixel);

            count -= (n - xx);
            xx += (n - xx);
            if (xx >= ww) {
              xx = 0;
              yy += 4;
              if (yy >= hh) goto done;
              }
            }
          break;
        }


      case op_color1:
      case op_color1_n:
        {
          if (opcode == op_color1_n) {
            store >> value;
            count = 1 + value;
            }

          store >> index;
          const std::uint8_t value = pal[index];

          count *= 4;
          while(count > 0) {

            const int n = std::min((xx + count), ww);

            for (int x = xx; x < n; x++)
              dest[(yy * stride) + x] = value;
            for (int x = xx; x < n; x++)
              dest[((1 + yy) * stride) + x] = value;
            for (int x = xx; x < n; x++)
              dest[((2 + yy) * stride) + x] = value;
            for (int x = xx; x < n; x++)
              dest[((3 + yy) * stride) + x] = value;

            count -= (n - xx);
            xx += (n - xx);
            if (xx >= ww) {
              xx = 0;
              yy += 4;
              if (yy >= hh) goto done;
              }
            }
          break;
        }

      case op_color2:
      case op_color2_n:
        {
          if (opcode == op_color2) {
            store >> c2[c2_index];
            index = c2_index;
            c2_index++;
            if (c2_index > 255) c2_index=0;
            }
          else {
            store >> index;
            }

          smc_pair_t & c = c2[index];

          std::uint16_t flags;
          for (int b = 0; b < count; b++) {
            store >> flags;

            for (int p=0;p<16;p++) {
              dest[(((p / 4) + yy) * stride) + xx + (p%4)] = pal[c[((flags>>(15-p))&0x1)]];
              }
            xx += 4;
            if (xx >= ww) {
              xx = 0;
              yy += 4;
              if (yy >= hh) goto done;
              }
            }
          break;
        }

      case op_color4:
      case op_color4_n:
        {
          if (opcode == op_color4) {
            store >> c4[c4_index];
            index = c4_index;
            c4_index++;
            if (c4_index > 255) c4_index=0;
            }
          else {
            store >> index;
            }

          smc_quad_t & c = c4[index];

          std::uint32_t flags;
          for (int b = 0; b < count; b++) {
            store >> flags;
            for (int p = 0; p < 16; p++) {
              dest[(((p / 4) + yy) * stride) + xx + (p%4)] = pal[c[((flags>>(30-p-p))&0x2)]];
              }
            xx += 4;
            if (xx >= ww) {
              xx = 0;
              yy += 4;
              if (yy >= hh) goto done;
              }
            }
          break;
        }
      case 0xf:
        printf("unknown ??\n");
        break;
      default:
        printf("%.2hhx not implemented\n",opcode);
        throw -1;
      }
    }
done:
    return;
    }