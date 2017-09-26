#ifndef GAP_RINGBUFFER_H
#define GAP_RINGBUFFER_H

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <array>
#include <memory>

namespace gap {

// Simple Linear Buffer
class buffer {
public:
  struct part {
    std::byte* data = nullptr;
    std::uint32_t size = 0;
    };
private:
  std::unique_ptr<std::byte[]> data;
  std::uint32_t size = 2;
  std::uint32_t rpos;
  std::uint32_t wpos;
private:
  buffer(const buffer&) = delete;
  buffer& operator=(const buffer&) = delete;
private:
  buffer(buffer&&) = delete;
  buffer& operator=(buffer&&) = delete;
public:
  // Construct ringbuffer of size `sz`
  // Size is rounded up to the nearest power of two.
  buffer(std::uint32_t sz);

  void clear() { wpos = rpos = 0; }

  // Returns capacity of this ringbuffer
  std::uint32_t capacity() const noexcept;

  // Returns readable space in the ringbuffer
  std::uint32_t readable() const noexcept;

  // Returns rewindable space in buffer
  std::uint32_t rewindable() const noexcept;

  // Returns writable space in the ringbuffer
  std::uint32_t writeable() const noexcept;

  // Copy 'n' bytes from 'src' into ringbuffer
  // Returns number of bytes copied.
  std::uint32_t write(const std::byte* src, std::uint32_t n) noexcept;

  // Copies `n` bytes from ringbuffer into 'dst`.
  // Returns number of bytes copied.
  std::uint32_t read(std::byte* dst, std::uint32_t n) noexcept;

  // Copies `n` bytes from ringbuffer into `dst` without advancing the read position.
  // Returns number of bytes copied.
  std::uint32_t peek(std::byte* dst, std::uint32_t n) const noexcept;

  void rewind(std::uint32_t n) noexcept;

  // Advance read position by `n` bytes
  void read(std::uint32_t n) noexcept;

  // Advance write position by 'n' bytes
  void wrote(std::uint32_t n) noexcept;

  inline std::byte & operator[](std::uint32_t index) const { return data[rpos + index]; }

  inline std::byte* readptr() const noexcept { return data.get() + rpos; }

  std::byte* writeptr(std::uint32_t &) noexcept;


  void hexdump(std::uint32_t n) {
    printf("hex(%u):", n);
    for (int i=0;i<std::min(n,readable());i++)
      printf("%.2hhx ",(unsigned char)(*this)[i]);
    printf("\n");
    }

  // find c in buffer
  int find_first_of(char c);

  // Destructor
  ~buffer();
  };

}

#endif
