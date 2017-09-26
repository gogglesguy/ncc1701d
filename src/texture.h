#ifndef TEXTURE_H
#define TEXTURE_H

#include <epoxy/gl.h>

namespace gap {

class texture {
public:
  unsigned int id = 0;
  float cw = 1.0f;
  float ch = 1.0f;
  float aspect = 1.0f;
public:

  void set(const void * image, int width = 0, int height = 0, int format = GL_BGRA);

  void select();

  ~texture();
  };

}

#endif
