#include "texture.h"

namespace gap {

texture::~texture() {
  }

void texture::select() {
  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
  glBindTexture(GL_TEXTURE_2D,id);
  }

void texture::set(const void * image, int image_width, int image_height, int format) {
  int texture_width;
  int texture_height;
  int texture_max;
  int type;
  int internalformat;

  switch(format) {
    case GL_BGRA: type = GL_UNSIGNED_INT_8_8_8_8_REV; internalformat = GL_RGBA8; break;
    case GL_RED: type = GL_UNSIGNED_BYTE; internalformat = GL_R8; break;
    default: throw -1;
    }

  if (image && image_width > 0 && image_height > 0) {

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_max);

    if ((image_width > texture_max) || (image_height > texture_max))
      throw -1;

    aspect = (float)image_width / (float)image_height;

    if (epoxy_has_gl_extension("GL_ARB_texture_non_power_of_two")) {
      texture_width  = image_width;
      texture_height = image_height;
      }
    else {
      texture_width = 1;
      texture_height = 1;
      while(image_width > texture_width) texture_width<<=1;
      while(image_height > texture_height) texture_height<<=1;

      if ((texture_width > texture_max) || (texture_height > texture_max))
        throw -1;
      }

    if (id==0) glGenTextures(1,&id);

    glBindTexture(GL_TEXTURE_2D,id);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    bool use_mipmap = (epoxy_gl_version() >= 30);

    if (use_mipmap) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }
    else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }

    if (texture_width == image_width && texture_height == image_height) {
      glTexImage2D(GL_TEXTURE_2D,0,internalformat,texture_width,texture_height,0,format,type,image);
      cw=ch=1.0f;
      }
    else {
      glTexImage2D(GL_TEXTURE_2D,0,internalformat,texture_width,texture_height,0,format,type,nullptr);
      glTexSubImage2D(GL_TEXTURE_2D,0,0,0,image_width,image_height,format,type,image);
      cw = (1.0f / (float)(texture_width))  * image_width;
      ch = (1.0f / (float)(texture_height)) * image_height;
      }
    if (use_mipmap)
      glGenerateMipmap(GL_TEXTURE_2D);
    }
  else {
    if (id) {
      glDeleteTextures(1,&id);
      id=0;
      }
    }
  }

}



