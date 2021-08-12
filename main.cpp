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
#include <cstring>
#include <cxxopts.hpp>
#include <stdexcept>
#include <unordered_map>
#include <vector>

enum CRCAlgorithm {
	CRC4_ITU,
	CRC5_EPC,
	CRC5_ITU,
	CRC5_USB,
	CRC6_CDMA2000A,
	CRC6_CDMA2000B,
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
	{"crc4_itu", CRCAlgorithm::CRC4_ITU},
	{"crc5_epc", CRCAlgorithm::CRC5_EPC},
	{"crc5_itu", CRCAlgorithm::CRC5_ITU},
	{"crc5_usb", CRCAlgorithm::CRC5_USB},
	{"crc6_cmda2000a", CRCAlgorithm::CRC6_CDMA2000A},
	{"crc6_cmda2000b", CRCAlgorithm::CRC6_CDMA2000B},
	{"crc6_itu", CRCAlgorithm::CRC6_ITU},
	{"crc6_nr", CRCAlgorithm::CRC6_NR},

	{"crc7", CRCAlgorithm::CRC7},
	{"crc8", CRCAlgorithm::CRC8},

	{"crc8_ebu", CRCAlgorithm::CRC8_EBU},
	{"crc8_maxim", CRCAlgorithm::CRC8_MAXIM},
	{"crc8_wcdma", CRCAlgorithm::CRC8_WCDMA},
	{"crc8_lte", CRCAlgorithm::CRC8_LTE},

	{"crc10", CRCAlgorithm::CRC10},
	{"crc10_cdma2000", CRCAlgorithm::CRC10_CDMA2000},
	{"crc11", CRCAlgorithm::CRC11},
	{"crc11_nr", CRCAlgorithm::CRC11_NR},
	{"crc12_cdma2000", CRCAlgorithm::CRC12_CDMA2000},
	{"crc12_dect", CRCAlgorithm::CRC12_DECT},
	{"crc12_umts", CRCAlgorithm::CRC12_UMTS},
	{"crc13_bcc", CRCAlgorithm::CRC13_BCC},
	{"crc15", CRCAlgorithm::CRC15},
	{"crc15_mpt1327", CRCAlgorithm::CRC15_MPT1327},
	{"crc16_arc", CRCAlgorithm::CRC16_ARC},
	{"crc16_buypass", CRCAlgorithm::CRC16_BUYPASS},
	{"crc16_ccittfalse", CRCAlgorithm::CRC16_CCITTFALSE},
	{"crc16_cdma2000", CRCAlgorithm::CRC16_CDMA2000},
	{"crc16_cms", CRCAlgorithm::CRC16_CMS},
	{"crc16_dectr", CRCAlgorithm::CRC16_DECTR},
	{"crc16_dectx", CRCAlgorithm::CRC16_DECTX},
	{"crc16_dnp", CRCAlgorithm::CRC16_DNP},
	{"crc16_genibus", CRCAlgorithm::CRC16_GENIBUS},
	{"crc16_kermit", CRCAlgorithm::CRC16_KERMIT},
	{"crc16_maxim", CRCAlgorithm::CRC16_MAXIM},
	{"crc16_modbus", CRCAlgorithm::CRC16_MODBUS},
	{"crc16_t10dif", CRCAlgorithm::CRC16_T10DIF},
	{"crc16_usb", CRCAlgorithm::CRC16_USB},
	{"crc16_x25", CRCAlgorithm::CRC16_X25},
	{"crc16_xmodem", CRCAlgorithm::CRC16_XMODEM},
	{"crc17_can", CRCAlgorithm::CRC17_CAN},
	{"crc21_can", CRCAlgorithm::CRC21_CAN},
	{"crc24", CRCAlgorithm::CRC24},
	{"crc24_flexraya", CRCAlgorithm::CRC24_FLEXRAYA},
	{"crc24_flexrayb", CRCAlgorithm::CRC24_FLEXRAYB},
	{"crc24_ltea", CRCAlgorithm::CRC24_LTEA},
	{"crc24_lteb", CRCAlgorithm::CRC24_LTEB},
	{"crc24_nrc", CRCAlgorithm::CRC24_NRC},

	{"crc30", CRCAlgorithm::CRC30},
	{"crc32", CRCAlgorithm::CRC32},
	{"crc32_bzip2", CRCAlgorithm::CRC32_BZIP2},
	{"crc32_c", CRCAlgorithm::CRC32_C},
	{"crc32_mpeg2", CRCAlgorithm::CRC32_MPEG2},
	{"crc32_posix", CRCAlgorithm::CRC32_POSIX},
	{"crc32_q", CRCAlgorithm::CRC32_Q},
	{"crc40_gsm", CRCAlgorithm::CRC40_GSM},
	{"crc64", CRCAlgorithm::CRC64},
	{"xor8", CRCAlgorithm::XOR8},
	{"xor8_masked", CRCAlgorithm::XOR8_MASK_MAJOR_BIT}};

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

template <typename T> static uint64_t computeCRC(CRCAlgorithm algorithm, const std::vector<T> &in) {
	const std::size_t nrBytes = in.size() * sizeof(T);
	const void *pData = in.data();

	switch (algorithm) {
	case CRC4_ITU: {
		static CRC::Table table(CRC::CRC_4_ITU());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC5_EPC: {
		static CRC::Table table(CRC::CRC_5_EPC());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC5_ITU: {
		static CRC::Table table(CRC::CRC_5_ITU());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC5_USB: {
		static CRC::Table table(CRC::CRC_5_USB());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC6_CDMA2000A: {
		static CRC::Table table(CRC::CRC_6_CDMA2000A());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC6_CDMA2000B: {
		static CRC::Table table(CRC::CRC_6_CDMA2000B());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC6_ITU: {
		static CRC::Table table(CRC::CRC_6_ITU());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC6_NR: {
		static CRC::Table table(CRC::CRC_6_NR());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC7: {
		static CRC::Table table(CRC::CRC_7());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC8: {
		static CRC::Table table(CRC::CRC_8());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC8_EBU: {
		static CRC::Table table(CRC::CRC_8_EBU());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC8_MAXIM: {
		static CRC::Table table(CRC::CRC_8_MAXIM());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC8_WCDMA: {
		static CRC::Table table(CRC::CRC_8_WCDMA());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC8_LTE: {
		static CRC::Table table(CRC::CRC_8_LTE());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC10: {
		static CRC::Table table(CRC::CRC_10());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC10_CDMA2000: {
		static CRC::Table table(CRC::CRC_10_CDMA2000());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC11: {
		static CRC::Table table(CRC::CRC_11());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC11_NR: {
		static CRC::Table table(CRC::CRC_11_NR());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC12_CDMA2000: {
		static CRC::Table table(CRC::CRC_12_CDMA2000());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC12_DECT: {
		static CRC::Table table(CRC::CRC_12_DECT());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC12_UMTS: {
		static CRC::Table table(CRC::CRC_12_UMTS());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC13_BCC: {
		static CRC::Table table(CRC::CRC_13_BBC());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC15: {
		static CRC::Table table(CRC::CRC_15());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC15_MPT1327: {
		static CRC::Table table(CRC::CRC_15_MPT1327());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_ARC: {
		static CRC::Table table(CRC::CRC_16_ARC());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_BUYPASS: {
		static CRC::Table table(CRC::CRC_16_BUYPASS());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_CCITTFALSE: {
		static CRC::Table table(CRC::CRC_16_CCITTFALSE());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_CDMA2000: {
		static CRC::Table table(CRC::CRC_16_CDMA2000());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_CMS: {
		static CRC::Table table(CRC::CRC_16_CMS());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_DECTR: {
		static CRC::Table table(CRC::CRC_16_DECTR());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_DECTX: {
		static CRC::Table table(CRC::CRC_16_DECTX());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_DNP: {
		static CRC::Table table(CRC::CRC_16_DNP());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_GENIBUS: {
		static CRC::Table table(CRC::CRC_16_GENIBUS());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_KERMIT: {
		static CRC::Table table(CRC::CRC_16_KERMIT());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_MAXIM: {
		static CRC::Table table(CRC::CRC_16_MAXIM());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_MODBUS: {
		static CRC::Table table(CRC::CRC_16_MODBUS());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_T10DIF: {
		static CRC::Table table(CRC::CRC_16_T10DIF());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_USB: {
		static CRC::Table table(CRC::CRC_16_USB());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_X25: {
		static CRC::Table table(CRC::CRC_16_X25());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC16_XMODEM: {
		static CRC::Table table(CRC::CRC_16_XMODEM());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC17_CAN: {
		static CRC::Table table(CRC::CRC_17_CAN());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC21_CAN: {
		static CRC::Table table(CRC::CRC_21_CAN());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC24: {
		static CRC::Table table(CRC::CRC_24());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC24_FLEXRAYA: {
		static CRC::Table table(CRC::CRC_24_FLEXRAYA());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC24_FLEXRAYB: {
		static CRC::Table table(CRC::CRC_24_FLEXRAYB());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC24_LTEA: {
		static CRC::Table table(CRC::CRC_24_LTEA());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC24_LTEB: {
		static CRC::Table table(CRC::CRC_24_LTEB());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC24_NRC: {
		static CRC::Table table(CRC::CRC_24_NRC());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC30: {
		static CRC::Table table(CRC::CRC_30());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC32: {
		static CRC::Table table(CRC::CRC_32());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC32_BZIP2: {
		static CRC::Table table(CRC::CRC_32_BZIP2());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC32_C: {
		static CRC::Table table(CRC::CRC_32_C());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC32_MPEG2: {
		static CRC::Table table(CRC::CRC_32_MPEG2());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC32_POSIX: {
		static CRC::Table table(CRC::CRC_32_POSIX());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC32_Q: {
		static CRC::Table table(CRC::CRC_32_Q());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC40_GSM: {
		static CRC::Table table(CRC::CRC_40_GSM());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case CRC64: {
		static CRC::Table table(CRC::CRC_64());
		return CRC::Calculate(pData, nrBytes, table);
	}
	case XOR8:
		return compute8Xor(in);
	case XOR8_MASK_MAJOR_BIT:
		return compute8Xor(in, 0x7F);
	default:
		assert(0);
		break;
	}
}

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
		options.add_options()("v,version", "Version information")("h,help", "helper information.")(
			"c,crc", "CRC Algorithm", cxxopts::value<std::string>()->default_value("crc8"))(
			"p,message-data-size", "Size of each messages in bytes.", cxxopts::value<uint32_t>()->default_value("5"))(
			"e,error-correction", "Perform Error Correction.", cxxopts::value<bool>()->default_value("false"))(
			"s,samples", "Samples", cxxopts::value<uint64_t>()->default_value("1000000"))(
			"t,tasks", "Task", cxxopts::value<int>()->default_value("2000"))(
			"b,nr-of-error-bits", "Number of bits error added to each message",
			cxxopts::value<int>()->default_value("1"))("f,forever", "Run it forever",
													   cxxopts::value<bool>()->default_value("false"));

		auto result = options.parse(argc, (char **&)argv);

		/*	If mention help, Display help and exit!	*/
		if (result.count("help") > 0) {
			std::cout << options.help();
			return EXIT_SUCCESS;
		}
		if (result.count("version") > 0) {
			std::cout << "Version: " << CRC_ANALYSIS_STR << " hash: " << CRC_ANALYSIS_GITCOMMIT_STR
					  << " branch: " << CRC_ANALYSIS_GITBRANCH_TR << std::endl;
			return EXIT_SUCCESS;
		}

		/*	*/
		dataSize = result["message-data-size"].as<uint32_t>();
		samples = result["samples"].as<uint64_t>();
		nrChunk = result["tasks"].as<int>();
		nrBitError = result["nr-of-error-bits"].as<int>();
		bool runForever = result["forever"].as<bool>();

		/*	*/
		const std::string &crcStr = result["crc"].as<std::string>();
		auto foundItem = table.find(crcStr);
		if (foundItem == table.end()) {
			std::cerr << "Invalid CRC Options " << crcStr << std::endl;
			return EXIT_FAILURE;
		}
		crcAlgorithm = (*foundItem).second;

		/*	*/
		uint32_t numTasks = nrChunk; /// marl::Thread::numLogicalCPUs();
		uint64_t numLocalSamplesPTask = samples / numTasks;
		assert(numLocalSamplesPTask > 0);

		/*	*/
		marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
		scheduler.bind();
		defer(scheduler.unbind()); // Automatically unbind before returning.

		do {

			// Create an event that is manually reset.
			marl::Event sayHello(marl::Event::Mode::Manual);
			// Create a WaitGroup with an initial count of numTasks.
			marl::WaitGroup saidHello(numTasks);

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

						/*	*/
						originalMsgCRC = computeCRC(crcAlgorithm, originalMsg);
						errorMsgCRC = computeCRC(crcAlgorithm, MsgWithError);

						/*	If message are not equal but the CRC are equal means that there was a incorrect CRC!	*/
						if (!isArrayEqual(originalMsg, MsgWithError) && originalMsgCRC == errorMsgCRC) {
							nrCollision++;
						}
					}

					/*	*/
					volatile const uint64_t _current_nr_samples = nrOfSamples.fetch_add(numLocalSamplesPTask);
					volatile const uint32_t _current_number_completed_task = nrTaskCompleted.fetch_add(1);
					volatile const uint64_t _current_nr_collision = nrCollision.load();

					/*	*/
					// assert(_current_nr_samples > 0);
					const double _collisionPerc = (double)_current_nr_collision / (double)_current_nr_samples;
					printf("\rCRC: %s, [%d/%d] NumberOfSamples %ld, collision: [%ld,%lf] nr-error-bit %d",
						   crcStr.c_str(), _current_number_completed_task, numTasks, _current_nr_samples,
						   _current_nr_collision, _collisionPerc, nrBitError);
					fflush(stdout);

					/*	*/
					defer(saidHello.done());
					sayHello.wait();

				});
			}

			sayHello.signal(); // Unblock all the tasks.

			saidHello.wait(); // Wait for all tasks to complete.
		} while (runForever);

		std::cout << std::endl;
	} catch (const std::exception &ex) {
		std::cerr << ex.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}