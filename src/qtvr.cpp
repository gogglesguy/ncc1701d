#include "qtvr.h"
#include "cinepak.h"
#include "smc.h"

int qtvr_panorama::node::load(gap::stream & store, atom & node) {
  switch(node.type()) {
    case fourcc("pHdr"):
      {
        store >> id;
        store >> hpan;
        store >> vpan;
        store >> zoom;
        store >> hrange[0];
        store >> vrange[0];
        store >> zrange[0];
        store >> hrange[1];
        store >> vrange[1];
        store >> zrange[1];
        store += 4;
        store += 4;
        store += 4;
        store += 4;
        break;
      }
    case fourcc("pLnk"):
      {
        std::int16_t count;
        std::uint16_t id;

        store += 2;
        store >> count;

        links.reserve(count);
        for (int i = 0; i < count; i++) {
          link lnk;
          store >> id;
          store += 2;
          store += 4;
          store += 4;
          store >> lnk.node;
          store += 12;
          store >> lnk.hpan;
          store >> lnk.vpan;
          store >> lnk.zoom;
          store += 4;
          store += 4;
          store += 4;
          store += 4;
          links.insert(std::make_pair(id, lnk));
          }
        break;
      }
    case fourcc("pHot"):
      {
          std::int16_t count;
          std::uint16_t id;

          store += 2;
          store >> count;

          spots.reserve(count);
          for (int i = 0; i < count; i++) {
            spot spt;

            store >> id;
            store += 2;
            store >> spt.type;
            store >> spt.data;
            store >> spt.hpan;
            store >> spt.vpan;
            store >> spt.zoom;
            store += 8; // rect
            store >> spt.mouse_over;
            store >> spt.mouse_down;
            store >> spt.mouse_up;
            store += 12;
            spots.insert(std::make_pair(id, spt));
            }
        break;
      }
    case fourcc("strT"):
      {
        strings.resize(node.size());
        store >> strings;
        break;
      }
    }
  return 0;
  }

void qtvr_panorama::node::debug() {
  for (auto & s: spots) {
    printf("spot %x\n",s.first);
    printf("\ttype: %s\n",s.second.type.text());
    printf("\tdata: %u\n",s.second.data);
    }

  for (auto & l: links) {
    printf("link %x\n",l.first);
    printf("\tnode: %u\n",l.second.node);
    }

  }






gap::stream & operator>>(gap::stream & store, mp4 & t) {
  store >> gap::stream::set_byteorder_be;
  atom::load(store, std::bind(&mp4::load, &t, std::placeholders::_1, std::placeholders::_2));
  return store;
  }




// -----------------------------------------------


int qtvr_panorama::load(gap::stream & store, atom & node) {
  switch(node.type()) {
    case fourcc("moov"): return node.level() == 0; break;
    case fourcc("trak"):
      {
        tracks.push_back(track());
        return 1;
      }
    case fourcc("tkhd"):
      {
        if (node.level() != 2) return 0;
        if (tracks.size() == 0) return 0;
        store += 12;
        store >> tracks.back().id;
        break;
      }
    case fourcc("mdia"): return node.level() == 2; break;
    case fourcc("minf"): return node.level() == 3; break;
    case fourcc("stbl"): return node.level() == 4; break;
    case fourcc("gmhd"): return node.level() == 4; break;
    case fourcc("STpn"): return node.level() == 5; break;
    case fourcc("pInf"):
      {
        std::uint8_t slen;
        std::uint16_t count;
        std::uint32_t id;
        std::int32_t time;

        // Pascal String -  Str32
        store >> slen;
        name.resize(slen);
        store >> name;
        store += (31 - slen);

        store >> default_node;
        store >> default_zoom;
        store += 4; // reserved
        store += 2; // pad
        store >> count;

        node_time.reserve(count);
        for (int c = 0; c < count; c++) {
          store >> id;
          store >> time;
          node_time.insert({ id, time });
          }
        pano_track = tracks.size() - 1;
        break;
      }
    case fourcc("pano"):
      {
        std::uint16_t major_version;
        std::uint16_t minor_version;
        if (node.size() != 144) break;
        store += 8;  // reserved
        store >> major_version;
        store >> minor_version;
        if (major_version!=0 && minor_version!=0) break;
        store >> image.track;
        store += 4;  // lowres track
        store += 24; // reserved
        store >> spot.track;
        store += 36; // reserved
        store >> hrange;
        store >> vrange;
        store >> zrange;
        store >> image.width;
        store >> image.height;
        store += 4;  // number of frames (eg nx * ny)
        store += 2;  // reserved
        store >> image.nx;
        store >> image.ny;
        store >> image.depth;
        store >> spot.width;
        store >> spot.height;
        store += 2;  // reserved
        store >> spot.nx;
        store >> spot.ny;
        store >> spot.depth;
        break;
      }
    default: return mp4::load(store, node); break;
    }
  return 0;
  }


gap::stream & operator>>(gap::stream & store, qtvr_panorama & vr) {
  store >> gap::stream::set_byteorder_be;
  atom::load(store, std::bind(&qtvr_panorama::load, &vr, std::placeholders::_1, std::placeholders::_2));
  if (vr.pano_track >= 0) {

    // Find the Image Track
    for (int t = 0; t < vr.tracks.size(); t++) {
      if (vr.tracks[t].id == vr.image.track) {
        vr.image.track = t;
        break;
        }
      }

    // Find the Spot Track
    for (int t = 0; t < vr.tracks.size(); t++) {
      if (vr.tracks[t].id == vr.spot.track) {
        vr.spot.track = t;
        break;
        }
      }

    printf("image track = %d\n",vr.image.track);
    printf("spot track = %d\n",vr.spot.track);

    vr.nodes.reserve(vr.panorama_track().sample_count());
    for (int sample = 0; sample < vr.panorama_track().sample_count(); sample++) {
      store.position(vr.panorama_track().sample_offset(sample));
      printf("position %llx\n",vr.panorama_track().sample_offset(sample));

      qtvr_panorama::node n;
      atom::load(store, vr.panorama_track().sample_size(sample), std::bind(&qtvr_panorama::node::load, &n, std::placeholders::_1, std::placeholders::_2));
      n.debug();
      vr.nodes.insert(std::make_pair(n.id, n));
      }
    }
  return store;
  }






void qtvr_panorama::load_image(gap::stream & store, std::uint32_t node, gap::image<std::uint32_t> & img ) {
  try {
    std::size_t sample = image_track().time_sample(node_time.at(node));
    img.resize(image.width, image.height);
    const int width = image.width / image.nx;
    const int height = image.height / image.ny;
    for (int tile = 0; tile < (image.nx * image.ny); tile++) {
      store.position(image_track().sample_offset(sample + tile));
      switch(image_track().codec) {
        case fourcc("cvid"):
          decode_cinepak_frame(store, img.data() + ((tile / image.nx) * image.nx * width * height) + ((tile % image.nx) * width), image.width);
          break;
        default: return;
        }
      }
    }
  catch(std::out_of_range) {
    img.resize(0,0);
    }
  }

void qtvr_panorama::load_spot(gap::stream & store, std::uint32_t node, gap::image<std::uint8_t> & img ) {
  try {
    std::size_t sample = spot_track().time_sample(node_time.at(node));
    img.resize(spot.width, spot.height);
    const int width = spot.width / spot.nx;
    const int height = spot.height / spot.ny;
    std::uint8_t * prev = nullptr;
    for (int tile = 0; tile < (spot.nx * spot.ny); tile++) {
      store.position(spot_track().sample_offset(sample + tile));
      switch(spot_track().codec) {
        case fourcc("smc "):
          {
            std::uint8_t * dst = img.data() + ((tile / spot.nx) * image.nx * width * height) + ((tile % spot.nx) * width);
            decode_smc_frame(store, prev, dst, width, height, spot.width);
            prev = dst;
            break;
          }
        default: return;
        }
      }
    }
  catch(std::out_of_range) {
    printf("failed to load spot for node %u\n", node);
    img.resize(0,0);
    }



  }

