#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include <CRC.h>

#include <cstdint> // Includes ::std::uint32_t
#include <cstring>
#include <iomanip>	// Includes ::std::hex
#include <iostream> // Includes ::std::cout
#include <vector>

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

typedef struct {
	uint64_t state;
	uint64_t inc;
} pcg32_random_t;
pcg32_random_t rng;

uint32_t pcg32_random_r(pcg32_random_t *rng) {
	uint64_t oldstate = rng->state;
	// Advance internal state
	rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void generateRandomMessage(std::vector<unsigned int> &data, int size) {
	for (int i = 0; i < size; i++) {
		data[i] = pcg32_random_r(&rng);
	}
}

void addRandomNoise(const std::vector<unsigned int> &in, std::vector<unsigned int> &out) {
	for (int i = 0; i < in.size(); i++) {
		out[i] = in[i] ^ (unsigned int)pcg32_random_r(&rng) + (unsigned int)(pcg32_random_r(&rng) * 0.0001f);
	}
}

uint32_t compute32Xor(const std::vector<unsigned int> &data) {
	uint32_t checksum = data[0];
	for (int i = 1; i < data.size(); i++) {
		checksum ^= data[i];
	}
	return checksum;
}

uint32_t compute8Xor(const std::vector<unsigned int> &data) {
	uint8_t *p = (uint8_t *)data.data();
	uint8_t checksum = p[0];
	for (int i = 1; i < data.size() * 4; i++) {
		checksum ^= p[i];
	}
	return checksum;
}

int main(int argc, const char **argv) {

	int samples = 10000000;
	int nrCollision = 0;

	std::vector<unsigned int> data(100);
	std::vector<unsigned int> noise(100);
	srand(time(NULL));

	for (int i = 0; i < samples; i++) {
		generateRandomMessage(data, 100);
		addRandomNoise(data, noise);

		std::uint32_t originalCRC = CRC::Calculate(data.data(), data.size() * sizeof(unsigned int), CRC::CRC_32());
		std::uint32_t noiseCRC = CRC::Calculate(noise.data(), noise.size() * sizeof(unsigned int), CRC::CRC_32());

		// std::uint32_t originalCRC = compute8Xor(data);
		// std::uint32_t noiseCRC = compute8Xor(noise);

		if (std::strncmp((const char *)data.data(), (const char *)noise.data(), data.size() * 4) != 0 &&
			originalCRC == noiseCRC) {

			nrCollision++;
		}

		// std::cout << std::hex << originalCRC << std::endl;
		// std::cout << std::hex << noiseCRC << std::endl;
	}
	printf("CRC8 collision %d, %f!\n", nrCollision, (float)nrCollision / (float)samples);
}