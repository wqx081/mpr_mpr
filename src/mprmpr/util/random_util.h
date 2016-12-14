#ifndef KUDU_UTIL_RANDOM_UTIL_H
#define KUDU_UTIL_RANDOM_UTIL_H

#include <cstdlib>
#include <stdint.h>

namespace mprmpr {

class Random;

void RandomString(void* dest, size_t n, Random* rng);
uint32_t GetRandomSeed32();

} // namespace mprmpr

#endif // KUDU_UTIL_RANDOM_UTIL_H
