#include <cassert>

#include "exceptions.h"
#include "io.h"

// Enable large file access on 32 bit platforms (see man feature_test_macros)
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <sys/types.h>  // open()
#include <sys/stat.h>   // open()
#include <fcntl.h>      // open() / fcntl
#include <unistd.h>     // close()
#include <climits>
#include <cctype>

namespace gap::io {


file::file(){
  }


file::file(const char * filename, io::direction direction) : dir(direction) {
  int flags = 0;

#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif

  switch(dir) {
    case direction::read: flags |= O_RDONLY; break;
    case direction::write: flags |= O_WRONLY; break;
    case direction::readwrite: flags |= O_RDWR; break;
    }

  handle = ::open(filename, flags);
  if (handle == -1)
    throw gap::exceptions::io();

#ifndef O_CLOEXEC
  flags = fcntl(handle, F_GETFD);
  if (fd == -1 || fcntl(handle, F_SETFD, flags|FD_CLOEXEC)==-1) {
    ::close(handle);
    handle = 0;
    throw gap::exceptions::io();
    }
#endif
  }


file::file(const std::string & filename, io::direction direction) : file(filename.c_str(), direction) {
  }


std::uint32_t file::read(std::byte* dst, std::uint32_t nbytes) {
  assert(handle >= 0);
  assert(nbytes < SSIZE_MAX);
  do {
    std::int32_t n = ::read(handle, dst, nbytes);
    if (n >= 0) return n;
    }
  while(errno == EINTR);
  throw gap::exceptions::io();
  }


std::uint32_t file::write(const std::byte* src, std::uint32_t nbytes) {
  assert(handle >= 0);
  assert(nbytes < SSIZE_MAX);
  do {
    std::int32_t n = ::write(handle, src, nbytes);
    if (n >= 0) return n;
    }
  while(errno == EINTR);
  throw gap::exceptions::io();
  }


void file::position(std::int64_t offset, origin from) {
  assert(handle >= 0);
  int whence;

  switch(from) {
    case origin::current: whence = SEEK_CUR; break;
    case origin::begin: whence = SEEK_SET; break;
    case origin::end: whence = SEEK_END; break;
    }

  if (lseek(handle, offset, whence) == -1)
    throw gap::exceptions::position();

  }


std::int64_t file::position() const {
  assert(handle >= 0);
  std::int64_t pos = lseek(handle, 0, SEEK_CUR);
  if (pos == -1)
    throw gap::exceptions::position();
  return pos;
  }

std::int64_t file::size() const {
  assert(handle >= 0);
  struct stat data;
  if (fstat(handle, &data) == 0)
    return data.st_size;
  else
    return -1;
  }


bool file::serial() const {
  return false;
  }


io::direction file::direction() const {
  return dir;
  }

file::~file() {
  assert(handle >= 0);
  close(handle);
  }



decorator::decorator(ioptr _io) : io(std::move(_io)) {}
}


