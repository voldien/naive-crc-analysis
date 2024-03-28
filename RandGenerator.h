#pragma once
#include "pcg_basic.h"
#include <cstdint>
#include <limits>
#include <random>
#include <stdlib.h>
#include <time.h>

class RandGenerator {
  public:
	virtual uint32_t getRandom() noexcept = 0;
	virtual float getRandomNormalized() noexcept = 0;
};

class PGSRandom : public RandGenerator {
  public:
	PGSRandom() {
		srand(time(nullptr));
		pcg32_srandom_r(&rng, rand(), rand());
	}
	uint32_t getRandom() noexcept override { return pcg32_random_r(&rng); }

	float getRandomNormalized() noexcept override {
		return static_cast<float>(this->getRandom()) * (1.0 / static_cast<float>(std::numeric_limits<uint32_t>::max()));
	}

  private:
	pcg32_random_t rng;
};

class UniformRandom : public RandGenerator {
  public:
	UniformRandom() {
		this->distribution = std::uniform_real_distribution<float>(0.0, 1.0);
		std::random_device rd; // Will be used to obtain a seed for the random number engine
		this->generator = std::mt19937(rd());
	}
	uint32_t getRandom() noexcept override {
		return static_cast<uint32_t>(static_cast<float>(std::numeric_limits<uint32_t>::max()) *
									 this->distribution(this->generator));
	}
	float getRandomNormalized() noexcept override { return this->distribution(this->generator); }

  private:
	std::uniform_real_distribution<float> distribution;
	std::mt19937 generator;
};