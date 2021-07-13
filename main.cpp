#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "RandGenerator.h"
#include <CRC.h>

#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/thread.h"
#include "marl/waitgroup.h"
#include <cassert>
#include <cstdint> // Includes ::std::uint32_t
#include <cstring>
#include <cxxopts.hpp>
#include <iomanip>	// Includes ::std::hex
#include <iostream> // Includes ::std::cout
#include <stdexcept>
#include <unordered_map>
#include <vector>

enum CRCAlgorithm {
	CRC8,
	XOR8,
	XOR8_MASK_MAJOR_BIT,
	CRC7,
};

static std::unordered_map<std::string, CRCAlgorithm> const table = {
	{"crc8", CRCAlgorithm::CRC8}, {"xor8", CRCAlgorithm::XOR8}, {"crc7", CRCAlgorithm::CRC7}};

template <typename T> void generateRandomMessage(std::vector<T> &data, unsigned int size, RandGenerator &gen) {
	for (unsigned int i = 0; i < size; i++) {
		data[i] = gen.getRandom();
	}
}

template <typename T> void addRandomNoise(const std::vector<T> &in, std::vector<T> &out, RandGenerator &gen) {
	for (unsigned int i = 0; i < in.size(); i++) {
		out[i] = in[i] ^ (T)gen.getRandom() + (T)(gen.getRandom() * 0.0001f);
	}
}

template <typename T>
void addBitError(const std::vector<T> &in, std::vector<T> &out, RandGenerator &gen, unsigned int nrBitError) {
	const uint32_t elementNrBits = sizeof(T) * 8;
	const uint32_t dataBitSize = in.size() * elementNrBits;

	assert(in.size() == out.size());

	// Copy the message
	out = in;

	for (unsigned int i = 0; i < nrBitError; i++) {

		const uint32_t bitIndex = gen.getRandom() % dataBitSize;
		const uint32_t arrayIndex = bitIndex / elementNrBits;
		const uint32_t bitFlipIndex = bitIndex % elementNrBits;
		assert(bitFlipIndex < elementNrBits);
		assert(arrayIndex < out.size());
		out[arrayIndex] ^= (1 << bitFlipIndex);
	}
}

void computeDiff(const std::vector<unsigned int> &in, std::vector<unsigned int> &out) {
	std::vector<unsigned int> p(in.size());
	for (unsigned int i = 0; i < in.size(); i++) {
		p[i] = out[i] ^ in[i];
	}
}

void perform_error_correction(const std::vector<unsigned int> &in, std::vector<unsigned int> &out) {}

template <typename T> uint32_t compute32Xor(const std::vector<T> &data) {
	uint32_t checksum = data[0];
	for (int i = 1; i < data.size(); i++) {
		checksum ^= data[i];
	}
	return checksum;
}

template <typename T> uint32_t compute8Xor(const std::vector<T> &data) {
	uint8_t *p = (uint8_t *)data.data();
	uint8_t checksum = p[0];
	for (int i = 1; i < data.size() * sizeof(T); i++) {
		checksum ^= p[i];
	}
	return checksum & 0x7F;
}

template <typename T> static bool isArrayEqual(const std::vector<T> &in, const std::vector<T> &out) {
	assert(in.size() == out.size());

	for (int i = 0; i < in.size(); i++) {
		if (in[i] != out[i])
			return false;
	}
	return true;
}

int main(int argc, const char **argv) {

	/*	*/
	try {
		uint64_t samples;
		uint32_t dataSize;
		uint32_t nrChunk;
		uint32_t nrBitError;
		std::atomic_uint32_t nrCollision = 0;
		std::atomic_uint64_t nrOfSamples = 0;
		std::atomic_uint32_t nrTaskCompleted = 0;
		CRCAlgorithm crcAlgorithm;

		cxxopts::Options options("Naive CRC Analysis", "");
		options.add_options()("d,debug", "Enable debugging") // a bool parameter
			("i,integer", "Int param", cxxopts::value<int>())("v,verbose", "Verbose output",
															  cxxopts::value<bool>()->default_value("false"))(
				"c,crc", "CRC", cxxopts::value<std::string>()->default_value("crc8"))(
				"p,data-chunk-size", "DataChunk", cxxopts::value<int>()->default_value("5"))(
				"e,error-correction", "Perform Error Correction", cxxopts::value<bool>()->default_value("false"))(
				"s,samples", "Samples", cxxopts::value<uint64_t>()->default_value("1000000"))(
				"t,task-size", "Task", cxxopts::value<int>()->default_value("2000"))(
				"m,number-of-bit-error", "Number of bit error per message", cxxopts::value<int>()->default_value("1"));

		auto result = options.parse(argc, (char **&)argv);

		dataSize = result["data-chunk-size"].as<int>();
		samples = result["samples"].as<uint64_t>();
		nrChunk = result["task-size"].as<int>();
		nrBitError = result["number-of-bit-error"].as<int>();

		auto it = table.find(result["crc"].as<std::string>());
		if (it != table.end()) {
			// Invalid
		}
		crcAlgorithm = (*it).second;
		const std::string &crcStr = (*it).first;

		/*	*/
		uint32_t numTasks = marl::Thread::numLogicalCPUs() * nrChunk;
		uint64_t localsamples = samples / numTasks;

		/*	*/
		marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
		scheduler.bind();
		defer(scheduler.unbind()); // Automatically unbind before returning.

		// Create an event that is manually reset.
		marl::Event sayHello(marl::Event::Mode::Manual);
		// Create a WaitGroup with an initial count of numTasks.
		marl::WaitGroup saidHello(numTasks);

		for (uint32_t i = 0; i < numTasks; i++) {
			marl::schedule([&] { // All marl primitives are capture-by-value.
				std::vector<unsigned int> data(dataSize);
				std::vector<unsigned int> noise(dataSize);
				PGSRandom randGen;

				/*	*/
				for (uint64_t i = 0; i < localsamples; i++) {
					generateRandomMessage(data, dataSize, randGen);
					addBitError(data, noise, randGen, nrBitError);

					std::uint32_t originalCRC =
						CRC::Calculate(data.data(), data.size() * sizeof(unsigned int), CRC::CRC_7());
					std::uint32_t noiseCRC =
						CRC::Calculate(noise.data(), noise.size() * sizeof(unsigned int), CRC::CRC_7());

					// std::uint32_t originalCRC = compute8Xor(data);
					// std::uint32_t noiseCRC = compute8Xor(noise);

					if (!isArrayEqual(data, noise) && originalCRC == noiseCRC) {
						nrCollision++;
					}
				}
				// Decrement the WaitGroup counter when the task has finished.

				// Blocking in a task?
				// The scheduler will find something else for this thread to do.
				const uint64_t _current_nr_samples = nrOfSamples.fetch_add(localsamples);
				const uint32_t _current_number_completed_task = nrTaskCompleted.fetch_add(1);

				const double _collisionPerc = (double)nrCollision.load() / (double)_current_nr_samples;
				printf("\rCRC: %s, [%d/%d] NumberOfSamples %ld, collision: [%d,%lf]", crcStr.c_str(),
					   _current_number_completed_task, numTasks, _current_nr_samples, nrCollision.load(),
					   _collisionPerc);
				fflush(stdout);

				/*	*/
				defer(saidHello.done());
				sayHello.wait();

			});
		}

		const double collisionPerc = (double)nrCollision.load() / (double)samples;

		sayHello.signal(); // Unblock all the tasks.

		saidHello.wait(); // Wait for all tasks to complete.

		printf("\rCRC8 collision %d, %f!\n", nrCollision.load(), collisionPerc * 100);
	} catch (const std::exception &ex) {
		std::cerr << ex.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}