#include <producer/producer.h>
#include <sample_idl/sample_idl.h>
#include <core/logger.h>
#include <chrono>

using namespace FastQ;
using namespace FastQ::SampleIdl;

static const LogModule LM_APP {"FASTQ_TEST_APP"};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test producer app");

	int size = 128 * 10;
	Producer producer {"test.shm", size};
	producer.Start();

	SampleData data;
	for (int i = 0; i < sizeof(data.mData); i++)
	{
		data.mData[i] = i;
	}

	LOG(INFO, LM_APP, "writing data..");

	std::chrono::seconds duration(60);
	auto start = std::chrono::steady_clock::now();

	uint64_t writeCount = 0;
	while (std::chrono::steady_clock::now() - start < duration)
	{
		data.mId = writeCount;
		producer.Push(&data, sizeof(data));
		writeCount++;
		LOG(INFO, LM_APP, "wrote: %llu", writeCount);
	}

	LOG(INFO, LM_APP, "total write count: %llu", writeCount);

	return 1;
}