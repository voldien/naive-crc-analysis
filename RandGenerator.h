#pragma once
#include "pcg_basic.h"
#include <stdlib.h>
#include <time.h>
class RandGenerator {
  public:
	virtual uint32_t getRandom(void) noexcept = 0;
};

class PGSRandom : public RandGenerator {
  public:
	PGSRandom(void) {
		srand(time(nullptr));
		pcg32_srandom_r(&rng, rand(), rand());
	}
	virtual uint32_t getRandom(void) noexcept { return pcg32_random_r(&rng); }

  private:
	pcg32_random_t rng;
};