#ifndef GAP_IO_H
#define GAP_IO_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <array>
#include <vector>

namespace gap::io {


enum class direction {
  read,
  write,
  readwrite,
  };


enum class origin {
  begin,
  current,
  end
  };


struct base {

  virtual std::uint32_t read(std::byte*, std::uint32_t nbytes) = 0;

  virtual std::uint32_t write(const std::byte*, std::uint32_t nbytes) = 0;

  virtual io::direction direction() const = 0;

  virtual std::int64_t position() const = 0;

  virtual std::int64_t size() const { return -1; };

  virtual void position(std::int64_t offset, io::origin) = 0;

  virtual bool serial() const = 0;

  virtual void flush() {};

  virtual ~base() {};
  };

typedef std::unique_ptr<base> ioptr;




class file : public base {
protected:
  int handle = 0;
  io::direction dir = direction::read;
private:
  file();
  file(const file&) = delete;
  file& operator=(const file&) = delete;
public:
  file(const char * filename, io::direction dir=direction::read);

  file(const std::string & filename, io::direction dir=direction::read);

  std::uint32_t read(std::byte*, std::uint32_t nbytes) override;

  std::uint32_t write(const std::byte*, std::uint32_t nbytes) override;

  io::direction direction() const override;

  std::int64_t position() const override;

  std::int64_t size() const override;

  void position(std::int64_t offset, io::origin) override;

  bool serial() const override;

  ~file();
  };


class decorator : public base {
protected:
  ioptr io;
private:
  decorator();
  decorator(const decorator&) = delete;
  decorator& operator=(const decorator&) = delete;
public:
  decorator(ioptr io);
  };

}

#endif
