#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <vector>


namespace gap {

template<typename T>
class image {
protected:
  std::vector<T> d;
  std::uint32_t w = 0;
  std::uint32_t h = 0;
public:
  image(){}
  image(std::uint32_t ww, std::uint32_t hh) : d(ww * hh), w(ww), h(hh)  {}

  void resize(std::uint32_t nw, std::uint32_t nh) {
    if ((nw * nh) != (w * h)) {
      d.resize(nw*nh);
      w = nw;
      h = nh;
      }
    }

  std::uint32_t width() const { return w; }

  std::uint32_t height() const { return h; }

  T * data() { return d.data(); }

  const T * data() const { return d.data(); }

  explicit operator bool() { return (w>0) && (h>0); }
  };







}

#endif
