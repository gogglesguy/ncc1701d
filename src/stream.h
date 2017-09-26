#ifndef GAP_STREAM_H
#define GAP_STREAM_H

#include <functional>
#include <cstring> // memcmp

#include "io.h"
#include "buffer.h"

namespace gap {

enum class endian {
  be,
  le,
  big = be,
  little = le,
  };

class stream {
public:
  using origin = io::origin;
  using direction = io::direction;
protected:
  io::ioptr io;
  gap::buffer buffer;
  bool swap = false;
protected:
  void cache(std::uint32_t nbytes);
  void flush(std::uint32_t nbytes);
private:
  stream(io::ioptr);
  stream(const stream&) = delete;
  stream& operator=(const stream&) = delete;

public:
  static stream* create(const char * url);

public:
  // set stream byte order
  endian byteorder(endian);

  // get stream byte order
  endian byteorder() const;

  // set byte order using >> operator
  static stream & set_byteorder_be(stream &);
  static stream & set_byteorder_le(stream &);

  // Use RAII to set byte order
  class byteorder_scope {
  protected:
    stream & store;
    endian mode;
  public:
    byteorder_scope(stream & s,endian m) : store(s), mode(s.byteorder(m)) {}
    ~byteorder_scope() { store.byteorder(mode); }
    };

public:

  bool eof() const;

  // Current Size of stream or -1 if unknown
  std::int64_t size() const;

  // Current Position
  std::uint64_t position() const {
    return io->position() - buffer.readable();
    }

  // Change position of the stream
  void position(std::int64_t offset, origin whence=origin::begin);

  // Move position forward by count bytes
  stream& operator+=(std::int64_t count) {
    position(count,io::origin::current);
    return *this;
    }

  // Move position backward by count bytes
  stream& operator-=(std::int64_t count) {
    position(-count,io::origin::current);
    return *this;
    }

  class position_scope {
  protected:
    stream & store;
    std::int64_t pos;
  public:
    position_scope(stream & s, std::int64_t offset) : store(s), pos(s.position()) {
      store.position(offset);
      };

    ~position_scope() {
      store.position(pos);
      }
    };


public:
  stream& operator>>(std::uint64_t &);
  stream& operator<<(const std::uint64_t);
  stream& operator>>(std::uint32_t &);
  stream& operator<<(const std::uint32_t);
  stream& operator>>(std::uint16_t &);
  stream& operator<<(const std::uint16_t);
  stream& operator>>(std::uint8_t &);
  stream& operator<<(const std::uint8_t);
  stream& operator>>(std::int64_t &);
  stream& operator<<(const std::int64_t);
  stream& operator>>(std::int32_t &);
  stream& operator<<(const std::int32_t);
  stream& operator>>(std::int16_t &);
  stream& operator<<(const std::int16_t);
  stream& operator>>(std::int8_t &);
  stream& operator<<(const std::int8_t);

  stream& operator>>(char &);
  stream& operator>>(std::byte &);
  stream& operator<<(const std::byte);
  stream& operator>>(float &);
  stream& operator<<(const float);
  stream& operator>>(double &);
  stream& operator<<(const double);
  stream& operator>>(std::string &);
  stream& operator<<(const std::string &);

  template<typename T, std::size_t N>
  stream& operator>>(T (&array)[N]) {
    for (auto & item: array) { *this >> item;}
    return *this;
    }

  template<typename T, std::size_t N>
  stream& operator<<(const T (&array)[N]) {
    for (auto & item: array) { *this << item;}
    return *this;
    }

  template<typename T>
  stream& operator>>(std::vector<T> & vector) {
    for (auto & item: vector) { *this >> item;}
    return *this;
    }

public:

  void getline(std::string &);

public:

  // Search forward for given byte pattern
  template <size_t N>
  void sync(std::array<std::byte,N> pattern) {
    do {
      cache(pattern.size());
      for (std::uint32_t i = 0; i < buffer.readable() - pattern.size(); i++) {
        if (std::memcmp(&buffer[i], pattern.data(), pattern.size()) == 0) {
          buffer.read(i);
          return;
          }
        }
      buffer.read(buffer.readable() - pattern.size() + 1);
      }
    while(1);
    }

  // Search forward for given string
  void sync(const char*);

  // Search forward using the match function and number of bytes to match.
  void sync(std::uint8_t nbytes, std::function<bool (const std::byte*, std::uint8_t n)> match);

  // Do exact match at current stream position
  bool match(const char * pattern);

  // Do case insensitive match at current stream position
  bool casematch(const char * pattern);

  bool contains(const char * charset);

  void load(std::string &, std::uint32_t n);
  void load(std::byte*, std::uint32_t n);


  void load_uint24(std::uint32_t & value);
  void load_uint48(std::uint64_t & value);



  void save(const std::byte*, std::uint32_t n);


  void hexdump(std::uint32_t nlines);


  void dump(const std::string & filename, std::uint32_t nbytes);

  // Operator used by stream setters (set_byteorder_*)
  stream& operator>>(stream& (*__func)(stream&)) {
    return __func(*this);
    }

  // Operator used by stream setters (set_byteorder_*)
  stream& operator<<(stream& (*__func)(stream&)) {
    return __func(*this);
    }

  };

}
#endif
