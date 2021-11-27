#include <sample_idl/sample_idl.h>
#include <core/logger.h>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <array>

using namespace FastQ;
using namespace FastQ::SampleIdl;
using namespace std::chrono_literals;

static const LogModule LM_APP {"FASTQ_BENCHMARK"};

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " [run_duration]"<< std::endl;
		return 1;
	}

	std::chrono::seconds duration = std::chrono::seconds(std::atoi(argv[1]));
	LOG(INFO, LM_APP, "starting benchmarking app, doing memcopies for %d sec and measuring throughput",
		duration.count());

	SampleData data;
	for (int i = 0; i < sizeof(data.mData); i++)
	{
		data.mData[i] = i;
	}

	LOG(INFO, LM_APP, "doing memcopies..");
	auto start = std::chrono::steady_clock::now();

	std::array<uint8_t, sizeof(data)> buffer {};
	uint32_t copyCount = 0;
	while (std::chrono::steady_clock::now() - start < duration)
	{
		data.mId = copyCount;
		std::memcpy(buffer.data(), &data, sizeof(data));
		copyCount++;
	}

	LOG(INFO, LM_APP, "total memcopies: %llu", copyCount);
	double mbPerSec = (copyCount * sizeof(data) * 1.0) / (1024.0 * 1024.0) / duration.count();
	LOG(INFO, LM_APP, "[benchmark_metric] {mb_per_sec: %f}", mbPerSec);

	return 1;
}