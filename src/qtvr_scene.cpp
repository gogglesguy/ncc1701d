#define _USE_MATH_DEFINES
#include <cmath>
#include "qtvr_scene.h"
#include <GL/glu.h>



void qtvr_panorama_scene::select(int x, int y, int width, int height) {
  const auto & current = qtvr.nodes.at(currentnode);
  std::uint8_t value = pick(x, y, width, height);
  if (value) {
    try {
      printf("selected spot %x\n",value);
      const auto & spot = current.spots.at((std::uint32_t)value);
      if (spot.type == fourcc("link") && current.links.at((std::uint16_t)spot.data).node != currentnode) {
        currentnode = current.links.at((std::uint16_t)spot.data).node;
        qtvr.nodes.at(currentnode).debug(); 
        reload = true;
        }
      }
    catch(std::out_of_range) {
      printf("out of range %x\n",value);
      }
    }
  }


bool qtvr_panorama_scene::is_link(std::uint8_t value) {
/*
  if (value) {
    std::int32_t nhpan;
    node * c = qtvr.find_node(currentnode);
    std::uint32_t n = c->get_node_for_spot(value, nhpan);
    return n > 0;
    }
*/
  return true;
  }


void qtvr_panorama_scene::load_resources() {
  if (background.id == 0 || reload) {

    {
      gap::image<std::uint32_t> image;
      qtvr.load_image(*store, currentnode, image);
      if (image) {
        background.set(image.data(),image.width(),image.height(),GL_BGRA);
        }
    }

    {
      gap::image<std::uint8_t> spotimage;
      qtvr.load_spot(*store, currentnode, spotimage);
      if (spotimage) {
        spot.set(spotimage.data(),spotimage.width(),spotimage.height(),GL_RED);
        }
    }

    reload = false;
    }
  }


void qtvr_panorama_scene::load(const char * filename) {
  if (store) delete store;
  store = gap::stream::create(filename);
  *store >> qtvr;
  currentnode = qtvr.default_node;
  printf("\n\n\ncurrentnode %u\n",currentnode);
  qtvr.nodes.at(currentnode).debug();


  hpan = fmodf((360 - (qtvr.nodes.at(currentnode).hpan >> 16)) + 90.0f, 360.0f);
  }


std::uint8_t qtvr_panorama_scene::pick(int x, int y, int width, int height) {
  load_resources();

  float step = 0.1f;
  float radius = 1.0f;
  float tx = background.ch / (2.0f * M_PI);
  float ch = 2.0f * M_PI * radius * background.aspect;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(75.f,(double)width/(double)height,0.1f,15.0f);

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glTranslatef(0.5,0.5,0.0);
  glRotatef(90.f,0.0f,0.0f,1.0);
  glTranslatef(-0.5,-0.5,0.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(1.0f,1.0f,1.0f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);


  glRotatef(hpan ,0.0f, 1.0f, 0.0f);
  glTranslatef(0.0f, ch * -0.5f, 0.0f);

  spot.select();

  glBegin(GL_QUAD_STRIP);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glTexCoord2f(0.0f, background.cw);
  glVertex3f(1.0f, ch, 0.0f);
  for (float angle = step; angle < (2.0f * M_PI); angle += step) {
    float x = cosf(angle) * radius;
    float z = - sinf(angle) * radius;
    glTexCoord2f(tx * angle, 0.0f);
    glVertex3f(x, 0.0f, z);
    glTexCoord2f(tx * angle, background.cw);
    glVertex3f(x, ch, z);
    }
  glTexCoord2f(background.ch, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glTexCoord2f(background.ch, background.cw);
  glVertex3f(1.0f, ch, 0.0f);
  glEnd();


  std::uint8_t color;
  glReadPixels(x, height - y, 1,1,GL_RED, GL_UNSIGNED_BYTE, &color);
 // printf("got %x\n", color);
  return color;
  }


void qtvr_panorama_scene::render(int width, int height) {

  load_resources();

  float step = 0.1f;
  float radius = 1.0f;
  float tx = background.ch / (2.0f * M_PI);
  float ch = 2.0f * M_PI * radius * background.aspect;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(75.f,(double)width/(double)height,0.1f,15.0f);

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glTranslatef(0.5,0.5,0.0);
  glRotatef(90.f,0.0f,0.0f,1.0);
  glTranslatef(-0.5,-0.5,0.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(1.0f,1.0f,1.0f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);


  glRotatef(hpan,0.0f, 1.0f, 0.0f);
  glTranslatef(0.0f, ch * -0.5f, 0.0f);

  background.select();


  glBegin(GL_QUAD_STRIP);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glTexCoord2f(0.0f, background.cw);
  glVertex3f(1.0f, ch, 0.0f);
  for (float angle = step; angle < (2.0f * M_PI); angle += step) {
    float x = cosf(angle) * radius;
    float z = - sinf(angle) * radius;
    glTexCoord2f(tx * angle, 0.0f);
    glVertex3f(x, 0.0f, z);
    glTexCoord2f(tx * angle, background.cw);
    glVertex3f(x, ch, z);
    }
  glTexCoord2f(background.ch, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glTexCoord2f(background.ch, background.cw);
  glVertex3f(1.0f, ch, 0.0f);
  glEnd();
  }


void qtvr_panorama_scene::panview(float v) {
  hpan = fmodf(hpan+v,360.0f);
  printf("hpan %g\n", fmodf(hpan, 360.0f));
  }
