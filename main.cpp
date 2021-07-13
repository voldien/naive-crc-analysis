#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "RandGenerator.h"
#include <CRC.h>

#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/thread.h"
#include "marl/waitgroup.h"
#include <cstdint> // Includes ::std::uint32_t
#include <cstring>
#include <cxxopts.hpp>
#include <iomanip>	// Includes ::std::hex
#include <iostream> // Includes ::std::cout
#include <stdexcept>
#include <vector>

void generateRandomMessage(std::vector<unsigned int> &data, unsigned int size, RandGenerator &gen) {
	for (unsigned int i = 0; i < size; i++) {
		data[i] = gen.getRandom();
	}
}

void addRandomNoise(const std::vector<unsigned int> &in, std::vector<unsigned int> &out, RandGenerator &gen) {
	for (unsigned int i = 0; i < in.size(); i++) {
		out[i] = in[i] ^ (unsigned int)gen.getRandom() + (unsigned int)(gen.getRandom() * 0.0001f);
	}
}

void computeDiff(const std::vector<unsigned int> &in, std::vector<unsigned int> &out) {
	std::vector<unsigned int> p(in.size());
	for (unsigned int i = 0; i < in.size(); i++) {
		p[i] = out[i] ^ in[i];
	}
}

void perform_error_correction(const std::vector<unsigned int> &in, std::vector<unsigned int> &out) {}

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
	for (int i = 1; i < data.size() * sizeof(unsigned int); i++) {
		checksum ^= p[i];
	}
	return checksum & 0x7F;
}

int main(int argc, const char **argv) {

	/*	*/
	try {
		uint64_t samples;
		uint32_t dataSize;
		uint32_t nrChunk;
		std::atomic_uint32_t nrCollision = 0;
		std::atomic_uint64_t nrOfSamples = 0;
		std::atomic_uint32_t nrTaskCompleted = 0;

		cxxopts::Options options("Naive CRC Analysis", "");
		options.add_options()("d,debug", "Enable debugging") // a bool parameter
			("i,integer", "Int param", cxxopts::value<int>())("v,verbose", "Verbose output",
															  cxxopts::value<bool>()->default_value("false"))(
				"c,crc", "CRC", cxxopts::value<int>())("p,data-chunk-size", "DataChunk",
													   cxxopts::value<int>()->default_value("5"))(
				"e,error-correction", "Perform Error Correction", cxxopts::value<bool>()->default_value("false"))(
				"s,samples", "Samples", cxxopts::value<uint64_t>()->default_value("1000000"))(
				"t,task-size", "Task", cxxopts::value<int>()->default_value("2000"))(
				"m,number-of-bit-error", "Number of bit error per message", cxxopts::value<int>()->default_value("1"));

		auto result = options.parse(argc, (char **&)argv);

		dataSize = result["data-chunk-size"].as<int>();
		samples = result["samples"].as<uint64_t>();
		nrChunk = result["task-size"].as<int>();

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
					addRandomNoise(data, noise, randGen);

					std::uint32_t originalCRC =
						CRC::Calculate(data.data(), data.size() * sizeof(unsigned int), CRC::CRC_8());
					std::uint32_t noiseCRC =
						CRC::Calculate(noise.data(), noise.size() * sizeof(unsigned int), CRC::CRC_8());

					// std::uint32_t originalCRC = compute8Xor(data);
					// std::uint32_t noiseCRC = compute8Xor(noise);

					if (std::strncmp((const char *)data.data(), (const char *)noise.data(),
									 data.size() * sizeof(unsigned int)) != 0 &&
						originalCRC == noiseCRC) {
						nrCollision++;
					}
				}
				// Decrement the WaitGroup counter when the task has finished.

				// Blocking in a task?
				// The scheduler will find something else for this thread to do.
				uint64_t _current_nr_samples = nrOfSamples.fetch_add(localsamples);
				uint32_t _current_number_completed_task = nrTaskCompleted.fetch_add(1);

				const double _collisionPerc = (double)nrCollision.load() / (double)_current_nr_samples;
				printf("\rCRC: %s, [%d/%d] NumberOfSamples %ld, collision: [%d,%lf]", "CRC8",
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