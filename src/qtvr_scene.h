#ifndef QTVR_SCENE_H
#define QTVR_SCENE_H

#include "qtvr.h"
#include "texture.h"

class qtvr_panorama_scene {
protected:
  gap::stream * store = nullptr;
  qtvr_panorama qtvr;
  gap::texture  background;
  gap::texture  spot;
  std::uint32_t currentnode;
  float         hpan;
  bool          reload = false;
public:
  void load_resources();

  void load(const char * filename);

  void render(int width, int height);

  std::uint8_t pick(int x, int y, int width, int height);

  void  select(int x, int y, int width, int height);

  bool is_link(std::uint8_t spot);

  void panview(float v);
  };

#endif
