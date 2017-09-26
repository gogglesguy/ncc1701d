#include "fourcc.h"

gap::stream& operator>>(gap::stream& store, fourcc & t) {
  store >> t.value;
  return store;
  }

