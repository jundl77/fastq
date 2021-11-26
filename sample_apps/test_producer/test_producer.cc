#include <producer/producer.h>
#include <sample_idl/sample_idl.h>
#include <core/logger.h>
#include <chrono>
#include <thread>

using namespace FastQ;
using namespace FastQ::SampleIdl;
using namespace std::chrono_literals;

static const LogModule LM_APP {"FASTQ_TEST_APP"};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test producer app");

	int size = 1024 * 1024 * 10;
	Producer producer {"test.shm", size};
	producer.Start();

	SampleData data;
	for (int i = 0; i < sizeof(data.mData); i++)
	{
		data.mData[i] = i;
	}

	LOG(INFO, LM_APP, "writing data..");

	std::chrono::seconds duration(6000);
	auto start = std::chrono::steady_clock::now();

	uint32_t writeCount = 0;
	while (std::chrono::steady_clock::now() - start < duration)
	{
		data.mId = writeCount;
		producer.Push(1, &data, sizeof(data));
		writeCount++;
		LOG(INFO, LM_APP, "wrote: %llu", writeCount);
		//std::this_thread::sleep_for(1ms);
	}

	LOG(INFO, LM_APP, "total write count: %llu", writeCount);

	return 1;
}