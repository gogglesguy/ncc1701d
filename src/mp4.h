#ifndef ATOM_H
#define ATOM_H

#include <functional>
#include "fourcc.h"

class atom {
protected:
  std::int64_t data_offset = -1;
  std::int64_t data_size = 0;
  fourcc id;
  std::uint8_t header_size = 0;
  std::uint8_t node_level = 0;
public:
  // Custom Loader
  std::function<int (gap::stream &, atom &)> loader = nullptr;

  // Size of data in atom
  std::int64_t size() const { return data_size; }

  // End of this atom
  std::int64_t end() const { return data_offset + data_size; }

  // Level of this atom in respect to its root
  std::int8_t level() const { return node_level; }

  // Atom Type
  fourcc type() const { return id; }

  // load child atoms
  void traverse(gap::stream &, std::function<int (gap::stream &, atom &)> loader);

  // load atoms from stream up to size
  static void load(gap::stream &, std::int64_t size, std::function<int (gap::stream &, atom &)> loader);

  // load atoms from stream
  static void load(gap::stream &, std::function<int (gap::stream &, atom &)> loader);

  // debug atom
  void debug() const;

  friend gap::stream & operator>>(gap::stream &, atom &);
  };

extern gap::stream & operator>>(gap::stream &, atom &);


struct mp4 {

  struct track {

    // time to sample entry
    struct stts_item {
      std::uint32_t nsamples; // number of samples
      std::uint32_t duration; // duration of number of samples
      };

    // sample to chunk entry
    struct stsc_item {
      std::uint32_t first;    // first chunk containing nsamples
      std::uint32_t nsamples; // nsamples
      std::uint32_t id;       // sample description id
      };

    // Sample description
    struct sample_description {
      fourcc type;
      sample_description(fourcc t) : type(t) {}
      virtual ~sample_description();
      };

    // Video Description
    struct video_description : public sample_description {
      };

    std::uint32_t id = 0;
    std::uint32_t fixed_sample_size = 0;
    std::vector<std::int64_t> stco;   // chunk offset table
    std::vector<std::uint32_t> stsz;  // sample size table
    std::vector<stts_item> stts; // time to sample table
    std::vector<stsc_item> stsc; // sample to chunk table
    std::vector<sample_description*> stsd;

    fourcc codec;
  public:

    // Number of samples
    std::size_t sample_count() const;

    // Offset in file of the given sample
    std::int64_t sample_offset(std::size_t sample) const;

    // Size in bytes of the given sample
    std::uint32_t sample_size(std::size_t sample) const;

    // Get chunk for the given sample. Also return number of samples at the start of this chunk.
    std::size_t sample_chunk(std::size_t sample, std::size_t & nsamples) const;

    // Get sample for the given time
    std::size_t time_sample(std::uint32_t time) const;
    };

  std::vector<track> tracks;
  int load(gap::stream &, atom &);
  };


// Load from stream
extern gap::stream & operator>>(gap::stream & store, mp4 & m);

#endif


