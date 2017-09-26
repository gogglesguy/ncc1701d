#include <cassert> // std::assert
#include <cstring> // std::memcpy
#include "exceptions.h"
#include "buffer.h"

#include <iostream>

namespace gap {

buffer::buffer(std::uint32_t sz) : size(sz) {

  // round up to nearest power of two
  // for non-powers of two, the modulos operation `pos & (size-1)` will need to be adjusted.
  //while(size < sz) size <<= 1;

  try {
    data = std::make_unique<std::byte[]>(size);
    }
  catch(std::bad_alloc&) {
    throw gap::exceptions::resource();
    }

  // allocate data
  //data = new (std::nothrow) std::byte[size];
  // if (data == nullptr) {
  //  throw gap::exceptions::resource();
  //  }

  rpos = 0;
  wpos = 0;
  }


buffer::~buffer() {
  //delete [] data;
  rpos = 0;
  wpos = 0;
  data = nullptr;
  }


std::uint32_t buffer::capacity() const noexcept {
  return size - 1;
  }

std::uint32_t buffer::readable() const noexcept {
  return wpos - rpos;
  }

std::uint32_t buffer::rewindable() const noexcept {
  return rpos;
  }

std::uint32_t buffer::writeable() const noexcept {
  return (size - (wpos - rpos));
  }


std::uint32_t buffer::write(const std::byte* src, std::uint32_t nbytes) noexcept {
  std::uint32_t ncopy = std::min(nbytes, writeable());
  if (ncopy) {
    if (ncopy > size - wpos) {
      std::uint32_t n = readable();
      std::memmove(data.get(), data.get() + rpos, n);
      rpos -= n;
      wpos -= n;
      }
    std::memcpy(data.get() + wpos, src, ncopy);
    wpos += ncopy;
    }
  return ncopy;
  }


std::uint32_t buffer::read(std::byte* dst, std::uint32_t nbytes) noexcept {
  std::uint32_t ncopy = std::min(nbytes, readable());
  if (ncopy) {
    std::memcpy(dst, data.get() + rpos, ncopy);
    rpos += ncopy;
    }
  return ncopy;
  }


std::uint32_t buffer::peek(std::byte* dst, std::uint32_t nbytes) const noexcept {
  std::uint32_t ncopy = std::min(nbytes, readable());
  if (ncopy) {
    std::memcpy(dst, data.get() + rpos, ncopy);
    }
  return ncopy;
  }


void buffer::rewind(std::uint32_t nbytes) noexcept {
  std::uint32_t n = std::min(nbytes, rewindable());
  if (n) rpos -= n;
  }

void buffer::read(std::uint32_t nbytes) noexcept {
  std::uint32_t n = std::min(nbytes, readable());
  if (n) rpos += n;
  }


void buffer::wrote(std::uint32_t nbytes) noexcept {
  std::uint32_t n = std::min(nbytes, writeable());
  if (n) wpos += n;
  assert(wpos <= size);
  }

std::byte* buffer::writeptr(std::uint32_t & nbytes) noexcept {
  nbytes = std::min(nbytes, writeable());
  if (nbytes > size - wpos) {
    int n = readable();
    if (n) {
      std::memmove(data.get(), data.get() + rpos, n);
      wpos=n;
      rpos=0;
      }
    else {
      wpos=rpos=0;
      }
    }
  return data.get() + wpos;
  }


int buffer::find_first_of(char c) {
  for (int pos = rpos; pos < wpos; pos++) {
    if (data[pos]==static_cast<std::byte>(c))
      return pos-rpos;
    }
  return -1;
  }


}
