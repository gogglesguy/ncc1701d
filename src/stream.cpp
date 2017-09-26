#include <cassert> // casser
#include <cstring> // strlen / memcmp
//#include <strings.h> // strncasecmp
#include "exceptions.h"
#include "stream.h"



namespace gap {

stream::stream(io::ioptr _io) : io(std::move(_io)), buffer(4096) {}

std::int64_t stream::size() const {
  return io->size();
  }

void stream::load(std::string & str, std::uint32_t n) {
  str.resize(n);
  *this >> str;
  }


endian stream::byteorder() const {
#ifdef GAP_BIG_ENDIAN
  return (swap) ? endian::le : endian::be;
#else
  return (swap) ? endian::be : endian::le;
#endif
  }


endian stream::byteorder(endian b) {
#ifdef GAP_BIG_ENDIAN
  endian prev = (swap) ? endian::le : endian::be;
  swap = (b==endian::le);
#else
  endian prev = (swap) ? endian::be : endian::le;
  swap = (b==endian::be);
#endif
  return prev;
  }


stream& stream::set_byteorder_be(stream& store){
  store.byteorder(endian::big);
  return store;
  }

stream& stream::set_byteorder_le(stream& store){
  store.byteorder(endian::little);
  return store;
  }


bool stream::eof() const {
  return size() >= 0 && position() == size();
  }


void stream::position(std::int64_t offset, io::origin whence) {
  if (whence == io::origin::current) {
    if (offset > 0) {
      if (offset <= buffer.readable()) {
        buffer.read(offset);
        return;
        }
      else {
        offset -= buffer.readable();
        buffer.clear();
        }
      }
    else if (offset < 0) {
      if (-offset <= buffer.rewindable()) {
        buffer.rewind(-offset);
        return;
        }
      else {
        offset -= buffer.readable();
        buffer.clear();
        }
      }
    else {
      return;
      }
    }
/*
 else {

    if (whence == io::origin::end)
      offset += size();

    std::int64_t start = position();
    std::int64_t end = start + buffer.readable();
    if (offset >= start && offset<end) {
      buffer.read(offset-start);
      return;
      }
    }
*/
  buffer.clear();
  io->position(offset,whence);
  //printf("new pos %llx %llx\n",io->position());
  }


stream* stream::create(const char * url) {
  return new stream(std::make_unique<io::file>(url));
  }


void stream::cache(std::uint32_t nbytes) {
  assert(io->direction() != direction::write);
  if (nbytes == 0 || nbytes > buffer.readable()) {
    std::uint32_t n = buffer.writeable();
    std::byte * data = buffer.writeptr(n);
    n = io->read(data, n);
    buffer.wrote(n);
    if (nbytes > 0 && buffer.readable() < nbytes)
      throw gap::exceptions::eof();
    if (nbytes == 0 && buffer.readable() == 0)
      throw gap::exceptions::eof();
    }
  }

//        store.hexdump((size+15)/16);

void stream::hexdump(std::uint32_t nbytes) {
  std::uint32_t maxlines = buffer.capacity() / 16;
  std::uint32_t nlines = (nbytes+15) / 16;
  nlines = std::min(maxlines, nlines);
  cache(16 * nlines);
  for (int n = 0; n < nlines; n++) {
    printf("%.8x  ",position() + (16 * n));
    for (int x = 0; x < 4; x++) {
      for (int b=0;b<4;b++) {
        printf("%.2hhx ",buffer[b+(x*4)+(16*n)]);
        }
      printf(" ");
      }
    printf("|");
    for (int b=0;b<16;b++) {
      if (std::isprint(static_cast<unsigned char>(buffer[b+(16*n)])))
        printf("%c",buffer[b+(16*n)]);
      else
        printf(".");
      }
    printf("|\n");
    }
  }


void stream::dump(const std::string & filename, std::uint32_t nbytes) {
  FILE * fp = fopen(filename.data(),"w+");
  if (fp) {
    while(nbytes) {
      cache(0);
      std::uint32_t count = std::min(buffer.readable(),nbytes);
      fwrite(buffer.readptr(),1,count,fp);
      nbytes -= count;
      buffer.read(count);
      }
    }
  fclose(fp);
  }

void stream::getline(std::string & line) {
  line.erase();
  do {
    cache(0);
    for (std::uint32_t i = 0; i < buffer.readable(); ++i) {
      if (static_cast<const char>(buffer[i]) == '\n') {
        line.append(reinterpret_cast<const char*>(buffer.readptr()), i);
        buffer.read(i+1);
        if (line.back() == '\r') line.pop_back();
        return;
        }
      }
    line.append(reinterpret_cast<const char*>(buffer.readptr()), buffer.readable());
    buffer.read(buffer.readable());
    }
  while(!eof());
  }







void stream::sync(const char * pattern) {
  assert(std::strlen(pattern) > 0);
  assert(std::strlen(pattern) < 256);
  std::uint8_t nbytes = std::strlen(pattern);
  do {
    cache(nbytes);
    for (int i=0;i<buffer.readable()-nbytes;i++) {
      if (std::memcmp(&buffer[i], pattern, nbytes) == 0) {
        buffer.read(i);
        return;
        }
      }
    buffer.read(buffer.readable()-nbytes+1);
    }
  while(1);
  }


void stream::sync(std::uint8_t nbytes, std::function<bool (const std::byte*, std::uint8_t n)> match) {
  do {
    cache(nbytes);
    for (int i=0;i<buffer.readable()-nbytes;i++) {
      if (match(&buffer[i],nbytes)) {
        buffer.read(i);
        return;
        }
      }
    buffer.read(buffer.readable()-nbytes+1);
    }
  while(1);
  }

bool stream::match(const char * pattern) {
  assert(std::strlen(pattern) > 0);
  assert(std::strlen(pattern) < buffer.capacity());
  std::int32_t nbytes = std::strlen(pattern);
  cache(nbytes);
  return std::strncmp(reinterpret_cast<const char*>(buffer.readptr()), pattern, nbytes) == 0;
  }


bool stream::casematch(const char * pattern) {
  assert(std::strlen(pattern) > 0);
  assert(std::strlen(pattern) < buffer.capacity());
  std::int32_t nbytes = std::strlen(pattern);
  cache(nbytes);
  return strncasecmp(reinterpret_cast<const char*>(buffer.readptr()), pattern, nbytes) == 0;
  }


bool stream::contains(const char * charset) {
  cache(1);
  const char * x = std::strchr(charset, static_cast<int>(buffer[0]));
  return  (x != nullptr) && (*x != '\0');
  }




stream& stream::operator>>(std::string & value) {
  if (value.size() < buffer.capacity()) {
    cache(value.size());
    value.replace(0, value.size(), reinterpret_cast<const char*>(buffer.readptr()), value.size());
    buffer.read(value.size());
    }
  else {
    std::uint32_t n = buffer.readable();
    if (n) {
      value.replace(0, n, reinterpret_cast<const char*>(buffer.readptr()), n);
      buffer.read(value.size());
      }
    if (io->read(reinterpret_cast<std::byte*>(value.data() + n),value.size() - n) != (value.size() - n))
      throw gap::exceptions::eof();
    }
  return *this;
  }




stream& stream::operator>>(std::uint64_t & value) {
  cache(8);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[7] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[6] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[5] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[4] = buffer[3];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[4];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[5];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[6];
    reinterpret_cast<std::byte*>(&value)[0] = buffer[7];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[3];
    reinterpret_cast<std::byte*>(&value)[4] = buffer[4];
    reinterpret_cast<std::byte*>(&value)[5] = buffer[5];
    reinterpret_cast<std::byte*>(&value)[6] = buffer[6];
    reinterpret_cast<std::byte*>(&value)[7] = buffer[7];
    }
  buffer.read(8);
  return *this;
  }


stream& stream::operator>>(std::int64_t & value) {
  cache(8);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[7] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[6] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[5] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[4] = buffer[3];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[4];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[5];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[6];
    reinterpret_cast<std::byte*>(&value)[0] = buffer[7];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[3];
    reinterpret_cast<std::byte*>(&value)[4] = buffer[4];
    reinterpret_cast<std::byte*>(&value)[5] = buffer[5];
    reinterpret_cast<std::byte*>(&value)[6] = buffer[6];
    reinterpret_cast<std::byte*>(&value)[7] = buffer[7];
    }
  buffer.read(8);
  return *this;
  }


stream& stream::operator>>(std::uint32_t & value) {
  cache(4);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[3] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[0] = buffer[3];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[3];
    }
  buffer.read(4);
  return *this;
  }

stream& stream::operator>>(std::int32_t & value) {
  cache(4);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[3] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[0] = buffer[3];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[3];
    }
  buffer.read(4);
  return *this;
  }


stream& stream::operator>>(std::uint16_t & value) {
  cache(2);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[0];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    }
  buffer.read(2);
  return *this;
  }

stream& stream::operator>>(std::int16_t & value) {
  cache(2);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[0];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    }
  buffer.read(2);
  return *this;
  }


stream& stream::operator>>(std::uint8_t & value) {
  cache(1);
  reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
  buffer.read(1);
  return *this;
  }


stream& stream::operator>>(std::int8_t & value) {
  cache(1);
  reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
  buffer.read(1);
  return *this;
  }


stream& stream::operator>>(char & value) {
  cache(1);
  reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
  buffer.read(1);
  return *this;
  }



void stream::load_uint24(std::uint32_t & value) {
  cache(3);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[3] = (std::byte)(0);
    reinterpret_cast<std::byte*>(&value)[2] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[0] = buffer[2];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[3] = (std::byte)(0);
    }
  buffer.read(3);
  }


void stream::load_uint48(std::uint64_t & value) {
  cache(6);
  if (swap) {
    reinterpret_cast<std::byte*>(&value)[6] = (std::byte)(0);
    reinterpret_cast<std::byte*>(&value)[6] = (std::byte)(0);
    reinterpret_cast<std::byte*>(&value)[5] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[4] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[3];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[4];
    reinterpret_cast<std::byte*>(&value)[0] = buffer[5];
    }
  else {
    reinterpret_cast<std::byte*>(&value)[0] = buffer[0];
    reinterpret_cast<std::byte*>(&value)[1] = buffer[1];
    reinterpret_cast<std::byte*>(&value)[2] = buffer[2];
    reinterpret_cast<std::byte*>(&value)[3] = buffer[3];
    reinterpret_cast<std::byte*>(&value)[4] = buffer[4];
    reinterpret_cast<std::byte*>(&value)[5] = buffer[5];
    reinterpret_cast<std::byte*>(&value)[6] = (std::byte)(0);
    reinterpret_cast<std::byte*>(&value)[7] = (std::byte)(0);
    }
  buffer.read(6);
  }



}
