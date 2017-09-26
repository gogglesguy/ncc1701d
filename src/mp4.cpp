#include "mp4.h"


void atom::debug() const {
  printf("%s - %u\n", id.text(), data_size);
  }


gap::stream & operator>>(gap::stream & store, atom & node) {
  std::uint32_t size32;
  store >> size32;
  store >> node.id;
  if (size32 == 1)  {
    node.header_size = 12;
    store >> node.data_size;
    }
  else {
    node.header_size = 8;
    node.data_size = size32;
    }
  node.data_size -= node.header_size;
  node.data_offset = store.position();
  return store;
  }


void atom::traverse(gap::stream & store, std::function<int (gap::stream &, atom &)> loader) {
  atom::load(store, data_size - (store.position() - data_offset), loader);
  }


void atom::load(gap::stream & store, std::function<int (gap::stream &, atom &)> loader) {
  atom::load(store, store.size() - store.position(), loader);
  }


void atom::load(gap::stream & store, std::int64_t size, std::function<int (gap::stream &, atom &)> loader) {

  // Work in Big Endian
  store >> gap::stream::set_byteorder_be;

  const int maxlevel = 10;
  int level = 0;
  bool last = false;

  atom nodes[maxlevel + 1];

  nodes[level].data_size = size;
  do {
    //printf("atom::load at level %d - %lld\n",level, nodes[level].size);

    while (nodes[level].data_size >= 8 || nodes[level].data_size == -1) {

      //printf("next atom::load at level %d - %lld\n",level, nodes[level].size);

      // next node
      atom & node = nodes[level + 1];
      node.loader = nodes[level].loader;

      // read node
      node.node_level = level;
      store >> node;

      // nodes of size 0 indicates last node in streeam
      if (node.data_size == 0) {

        // only allowed at top level
        if (node.node_level > 0)
          throw -1;

        // set size of the node if known
        if (store.size() > 0)
          node.data_size = store.size() - store.position();

        // mark last
        last = true;
        }

      int next;
      if (nodes[level].loader)
        next = nodes[level].loader(store, node);
      else
        next = loader(store, node);

      if (next < 0 || last) { // Interrupted or last node in stream
        return;
        }
      else if (next == 0) { // skip this node

        // debug node
        for(int i=0;i<level;i++) printf("  ");
        printf("-");
        node.debug();


        nodes[level].data_size -= (node.data_size + node.header_size);
        store.position(node.end());
        }
      else { // next level

        // debug node
        for(int i=0;i<level;i++) printf("  ");
        printf("+");
        node.debug();

        nodes[level].data_size -= (node.data_size + node.header_size);
        level++;
        if (level > maxlevel)
          throw -1;
        nodes[level].data_size -= (store.position() - node.data_offset);
        }
      }

    // Account for any remaining data
    if (nodes[level].data_size > 0)
      store += nodes[level].data_size;

    nodes[level].loader = nullptr;
    }
  while(--level > 0);
  }





std::size_t mp4::track::sample_count() const {
  std::size_t count = 0;
  for (auto s: stts)
    count += s.nsamples;
  return count;
  }


std::uint32_t mp4::track::sample_size(std::size_t sample) const {
  if (fixed_sample_size)
    return fixed_sample_size;
  else
    return stsz[sample];
  }


std::int64_t mp4::track::sample_offset(std::size_t sample) const {
  std::int64_t offset;
  std::size_t nsamples;
  std::size_t chunk = sample_chunk(sample, nsamples);

  if (stco.size())
    offset = stco[std::min(chunk, stco.size() - 1)];
  else
    offset = 8;

  if (fixed_sample_size) {
    offset += (sample - nsamples) * fixed_sample_size;
    }
  else {
    for (;nsamples < sample; nsamples++)
      offset += stsz[nsamples];
    }
  return offset;
  }


std::size_t mp4::track::sample_chunk(std::size_t sample, std::size_t & nsamples) const {
  std::size_t samplecount = 0;

  for (int i = 0; i < stsc.size() - 1; i++) {

    // number of samples between this entry and the next entry.
    std::size_t n = (stsc[i + 1].first - stsc[i].first) * stsc[i].nsamples;

    // sample before next entry
    if (sample < samplecount + n) {

      // chunk number = first + (number samples left / number of samples per chunk) - 1
      std::size_t chunk = stsc[i].first + ((sample - samplecount) / stsc[i].nsamples) - 1;

      // number of samples at the start of this chunk
      nsamples = samplecount + (((chunk + 1) - stsc[i].first) * stsc[i].nsamples);

      return chunk;
      }

    samplecount += n;
    }

  // chunk number
  std::size_t chunk = stsc.back().first + ((sample - samplecount) / stsc.back().nsamples) - 1;

  // number of samples at the start of this chunk
  nsamples = samplecount + (((chunk + 1) - stsc.back().first) * stsc.back().nsamples);

  return chunk;
  }

std::size_t mp4::track::time_sample(std::uint32_t time) const {
  std::size_t  nsamples = 0;
  std::int64_t ncount = 0;

  time /= 10;

  for (auto & s: stts) {
    std::int64_t n = s.nsamples * s.duration;
    if (time < (ncount + n))
      return nsamples + ( (time - ncount) / s.duration);
    ncount += n;
    nsamples += s.nsamples;
    }

  return nsamples + 1;
  }


int mp4::load(gap::stream & store, atom & node) {
  std::uint32_t version_and_flags;
  std::uint32_t count;

  switch(node.type()) {

    case fourcc("co64"):
      {
        track & t = tracks.back();
        store >> version_and_flags;
        store >> count;
        t.stco.resize(count);
        for (auto & c: t.stco) {
          store >> c;
          }
        break;
      }

    case fourcc("stco"):
      {
        track & t = tracks.back();
        std::uint32_t offset;
        store >> version_and_flags;
        store >> count;
        t.stco.resize(count);
        for (auto & c: t.stco) {
          store >> offset;
          c = offset;
          }
        break;
      }

    case fourcc("stsz"):
      {
        track & t = tracks.back();
        store >> version_and_flags;
        store >> t.fixed_sample_size;
        store >> count;
        t.stsz.resize(count);
        for (auto & samplesize: t.stsz) {
          store >> samplesize;
          }
        break;
      }

    case fourcc("stts"):
      {
        track & t = tracks.back();
        store >> version_and_flags;
        store >> count;
        t.stts.resize(count);
        for (auto & item: t.stts) {
          store >> item.nsamples;
          store >> item.duration;
          }
        break;
      }

    case fourcc("stsc"):
      {
        track & t = tracks.back();
        store >> version_and_flags;
        store >> count;
        t.stsc.resize(count);
        for (auto & item: t.stsc) {
          store >> item.first;
          store >> item.nsamples;
          store >> item.id;
          }
        break;
      }

    case fourcc("stsd"):
      {
        store >> version_and_flags;
        store >> count;
        return 1;
        break;
      }

    case fourcc("cvid"):
    case fourcc("smc "):
      {
        track & t = tracks.back();
        if (node.level() != 6) return 0;
          t.codec = node.type();
        break;
      }


    }
  return 0;
  }






















