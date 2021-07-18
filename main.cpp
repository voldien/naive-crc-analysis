#define CRCPP_USE_CPP11
#define CRCPP_INCLUDE_ESOTERIC_CRC_DEFINITIONS
#include "RandGenerator.h"
#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/thread.h"
#include "marl/waitgroup.h"
#include "revision.h"
#include <CRC.h>
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
	CRC4_ITU,
	CRC5_EPC,
	CRC5_ITU,
	CRC5_USB,
	CRC6_CDMA2000A,
	CRC5_CDMA2000B,
	CRC6_ITU,
	CRC6_NR,
	CRC7,
	CRC8,
	CRC8_EBU,
	CRC8_MAXIM,
	CRC8_WCDMA,
	CRC8_LTE,
	CRC10,
	CRC10_CDMA2000,
	CRC11,
	CRC11_NR,
	CRC12_CDMA2000,
	CRC12_DECT,
	CRC12_UMTS,
	CRC13_BCC,
	CRC15,
	CRC15_MPT1327,
	CRC16_ARC,
	CRC16_BUYPASS,
	CRC16_CCITTFALSE,
	CRC16_CDMA2000,
	CRC16_CMS,
	CRC16_DECTR,
	CRC16_DECTX,
	CRC16_DNP,
	CRC16_GENIBUS,
	CRC16_KERMIT,
	CRC16_MAXIM,
	CRC16_MODBUS,
	CRC16_T10DIF,
	CRC16_USB,
	CRC16_X25,
	CRC16_XMODEM,
	CRC17_CAN,
	CRC21_CAN,
	CRC24,
	CRC24_FLEXRAYA,
	CRC24_FLEXRAYB,
	CRC24_LTEA,
	CRC24_LTEB,
	CRC24_NRC,
	CRC30,
	CRC32,
	CRC32_BZIP2,
	CRC32_C,
	CRC32_MPEG2,
	CRC32_POSIX,
	CRC32_Q,
	CRC40_GSM,
	CRC64,
	XOR8,
	XOR8_MASK_MAJOR_BIT,
};

static std::unordered_map<std::string, CRCAlgorithm> const table = {
	{"crc7", CRCAlgorithm::CRC7},

	{"crc8", CRCAlgorithm::CRC8},	{"crc10", CRCAlgorithm::CRC10},
	{"crc11", CRCAlgorithm::CRC11}, {"crc15", CRCAlgorithm::CRC15},
	{"crc24", CRCAlgorithm::CRC24}, {"crc30", CRCAlgorithm::CRC30},
	{"crc32", CRCAlgorithm::CRC32}, {"crc64", CRCAlgorithm::CRC64},
	{"xor8", CRCAlgorithm::XOR8},	{"xor8_masked", CRCAlgorithm::XOR8_MASK_MAJOR_BIT}};

template <typename T> void generateRandomMessage(std::vector<T> &data, unsigned int size, RandGenerator &gen) {
	assert(data.size() <= size);
	for (size_t i = 0; i < size; i++) {
		data[i] = gen.getRandom();
	}
}

template <typename T>
void setFlippedBitErrors(const std::vector<T> &in, std::vector<T> &out, RandGenerator &gen, unsigned int nrBitError) {
	const uint32_t elementNrBits = sizeof(T) * 8;
	const uint32_t dataBitSize = in.size() * elementNrBits;

	assert(in.size() == out.size());

	// Copy the message
	out = in;

	for (unsigned int i = 0; i < nrBitError; i++) {

		const uint32_t bitIndex = gen.getRandom() % dataBitSize;

		/*	Convert a bit index to array index and bit offset.	*/
		const uint32_t arrayIndex = bitIndex / elementNrBits;
		const uint32_t bitFlipIndex = bitIndex % elementNrBits;

		/*	*/
		assert(bitFlipIndex < elementNrBits);
		assert(arrayIndex < out.size());

		/*	Flip a single bit.	*/
		out[arrayIndex] ^= (1 << bitFlipIndex);
	}
}

void computeDiff(const std::vector<unsigned int> &in, std::vector<unsigned int> &out) {
	std::vector<unsigned int> p(in.size());
	assert(in.size() == out.size());

	for (size_t i = 0; i < in.size(); i++) {
		/*	Compute the difference.	*/
		p[i] = out[i] ^ in[i];
	}
}

void attemptErrorCorrectMsg(const std::vector<unsigned int> &in, std::vector<unsigned int> &out) {}

template <typename T> static uint32_t compute32Xor(const std::vector<T> &data) {
	uint32_t checksum = data[0];
	for (int i = 1; i < data.size(); i++) {
		checksum ^= data[i];
	}
	return checksum;
}

template <typename T> static uint8_t compute8Xor(const std::vector<T> &data, uint8_t mask = 0xFF) {
	uint8_t *p = (uint8_t *)data.data();
	uint8_t checksum = p[0];
	const uint32_t nrBytes = data.size() * sizeof(T);

	for (size_t i = 1; i < nrBytes; i++) {
		checksum ^= p[i];
	}
	return checksum & mask;
}

template <typename T> static bool isArrayEqual(const std::vector<T> &in, const std::vector<T> &out) {
	assert(in.size() == out.size());

	for (size_t i = 0; i < in.size(); i++) {
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
		std::atomic_uint64_t nrCollision = 0;
		std::atomic_uint64_t nrOfSamples = 0;
		std::atomic_uint32_t nrTaskCompleted = 0;
		CRCAlgorithm crcAlgorithm;

		const std::string helperInfo = "Naive CRC Analysis\n"
									   "A simple program for checking error detection"
									   "";

		cxxopts::Options options("CRCAnalysis", helperInfo);
		options.add_options()("v,version", "Version information")("h,help", "helper information")(
			"c,crc", "CRC", cxxopts::value<std::string>()->default_value("crc8"))(
			"p,data-chunk-size", "DataChunk", cxxopts::value<uint32_t>()->default_value("5"))(
			"e,error-correction", "Perform Error Correction", cxxopts::value<bool>()->default_value("false"))(
			"s,samples", "Samples", cxxopts::value<uint64_t>()->default_value("1000000"))(
			"t,task-size", "Task", cxxopts::value<int>()->default_value("2000"))(
			"m,number-of-bit-error", "Number of bit error per message", cxxopts::value<int>()->default_value("1"));

		auto result = options.parse(argc, (char **&)argv);

		/*	If mention help, Display help and exit!	*/
		if (result.count("help") > 0) {
			std::cout << options.help();
			return EXIT_SUCCESS;
		}
		if (result.count("version") > 0) {
			std::cout << "Version: git-hash: " << CRC_ANALYSIS_GITCOMMIT_STR
					  << " - git-branch: " << CRC_ANALYSIS_GITBRANCH_TR << std::endl;
			return EXIT_SUCCESS;
		}

		/*	*/
		dataSize = result["data-chunk-size"].as<uint32_t>();
		samples = result["samples"].as<uint64_t>();
		nrChunk = result["task-size"].as<int>();
		nrBitError = result["number-of-bit-error"].as<int>();

		/*	*/
		const std::string &crcStr = result["crc"].as<std::string>();
		auto foundItem = table.find(crcStr);
		if (foundItem == table.end()) {
			std::cerr << "Invalid CRC Options " << crcStr << std::endl;
			return EXIT_FAILURE;
		}
		crcAlgorithm = (*foundItem).second;

		/*	*/
		uint32_t numTasks = marl::Thread::numLogicalCPUs() * nrChunk;
		uint64_t numLocalSamplesPTask = samples / numTasks;

		/*	*/
		marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
		scheduler.bind();
		defer(scheduler.unbind()); // Automatically unbind before returning.

		// Create an event that is manually reset.
		marl::Event sayHello(marl::Event::Mode::Manual);
		// Create a WaitGroup with an initial count of numTasks.
		marl::WaitGroup saidHello(numTasks);

		/*	Create lookup table.	*/
		// TODO

		for (uint32_t nthTask = 0; nthTask < numTasks; nthTask++) {
			marl::schedule([&] { // All marl primitives are capture-by-value.
				std::vector<unsigned int> originalMsg(dataSize);
				std::vector<unsigned int> MsgWithError(dataSize);
				PGSRandom randGen;

				/*	*/
				for (uint64_t i = 0; i < numLocalSamplesPTask; i++) {
					generateRandomMessage(originalMsg, dataSize, randGen);
					setFlippedBitErrors(originalMsg, MsgWithError, randGen, nrBitError);

					std::uint64_t originalMsgCRC, errorMsgCRC;
					switch (crcAlgorithm) {
					case CRC7:
						originalMsgCRC =
							CRC::Calculate(originalMsg.data(), originalMsg.size() * sizeof(unsigned int), CRC::CRC_7());
						errorMsgCRC = CRC::Calculate(MsgWithError.data(), MsgWithError.size() * sizeof(unsigned int),
													 CRC::CRC_7());
						break;
					case CRC8:
						originalMsgCRC =
							CRC::Calculate(originalMsg.data(), originalMsg.size() * sizeof(unsigned int), CRC::CRC_8());
						errorMsgCRC = CRC::Calculate(MsgWithError.data(), MsgWithError.size() * sizeof(unsigned int),
													 CRC::CRC_8());
						break;
					case CRC10:
						originalMsgCRC = CRC::Calculate(originalMsg.data(), originalMsg.size() * sizeof(unsigned int),
														CRC::CRC_10());
						errorMsgCRC = CRC::Calculate(MsgWithError.data(), MsgWithError.size() * sizeof(unsigned int),
													 CRC::CRC_10());
					case CRC11:
					case CRC15:
					case CRC24:
					case CRC30:
					case CRC32:
					case CRC64:
						break;
					case XOR8:
						originalMsgCRC = compute8Xor(originalMsg);
						errorMsgCRC = compute8Xor(MsgWithError);
						break;
					case XOR8_MASK_MAJOR_BIT:
						originalMsgCRC = compute8Xor(originalMsg, 0x7F);
						errorMsgCRC = compute8Xor(MsgWithError, 0x7F);
						break;

					default:
						assert(0);
					}

					/*	If message are not equal but the CRC are equal means that there was a incorrect CRC!	*/
					if (!isArrayEqual(originalMsg, MsgWithError) && originalMsgCRC == errorMsgCRC) {
						nrCollision++;
					}
				}

				/*	*/
				const uint64_t _current_nr_samples = nrOfSamples.fetch_add(numLocalSamplesPTask);
				const uint32_t _current_number_completed_task = nrTaskCompleted.fetch_add(1);
				const uint64_t _current_nr_collision = nrCollision.load();

				/*	*/
				const double _collisionPerc = (double)_current_nr_collision / (double)_current_nr_samples;
				printf("\rCRC: %s, [%d/%d] NumberOfSamples %ld, collision: [%ld,%lf] nr-error-bit %d", crcStr.c_str(),
					   _current_number_completed_task, numTasks, _current_nr_samples, _current_nr_collision,
					   _collisionPerc, nrBitError);
				fflush(stdout);

				/*	*/
				defer(saidHello.done());
				sayHello.wait();

			});
		}

		sayHello.signal(); // Unblock all the tasks.

		saidHello.wait(); // Wait for all tasks to complete.

		std::cout << std::endl;
	} catch (const std::exception &ex) {
		std::cerr << ex.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}