#ifndef QTVR_H
#define QTVR_H

#include <unordered_map>
#include "mp4.h"
#include "image.h"

struct qtvr_panorama : public mp4 {

  struct node {

    struct link {
      std::uint32_t node;
      std::int32_t hpan;
      std::int32_t vpan;
      std::int32_t zoom;
      };

    struct spot {
      fourcc type;
      std::uint32_t data;
      std::int32_t hpan;
      std::int32_t vpan;
      std::int32_t zoom;
      std::int32_t mouse_over;
      std::int32_t mouse_down;
      std::int32_t mouse_up;
      };

    std::uint32_t id;
    std::vector<char> strings;
    std::unordered_map<std::uint16_t, spot> spots;
    std::unordered_map<std::uint16_t, link> links;

    std::int32_t hpan;
    std::int32_t vpan;
    std::int32_t zoom;
    std::int32_t hrange[2];
    std::int32_t vrange[2];
    std::int32_t zrange[2];

    int load(gap::stream &, atom &);

    void debug();
    };

  std::unordered_map<std::uint32_t, node> nodes;
  std::unordered_map<std::uint32_t, std::int32_t> node_time;
  std::string name;
  std::uint32_t default_node;
  std::int32_t default_zoom;

  struct scene_info {
    std::int32_t track = -1;
    std::uint32_t width;
    std::uint32_t height;
    std::uint16_t depth;
    std::uint16_t nx;
    std::uint16_t ny;
    };
  scene_info image;
  scene_info spot;
  std::int32_t hrange[2];
  std::int32_t vrange[2];
  std::int32_t zrange[2];

  int pano_track = -1;

  int load(gap::stream &, atom &);


  const track & panorama_track() const { return tracks[pano_track]; }

  const track & image_track() const { return tracks[image.track]; }

  const track & spot_track() const { return tracks[spot.track]; }


  void load_image(gap::stream & store, std::uint32_t node, gap::image<std::uint32_t> & img);

  void load_spot(gap::stream & store, std::uint32_t node, gap::image<std::uint8_t> & img);
  };

// Load from stream
extern gap::stream & operator>>(gap::stream & store, qtvr_panorama & m);


#endif