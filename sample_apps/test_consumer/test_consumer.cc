#include <consumer/consumer.h>
#include <sample_idl/sample_idl.h>
#include <core/logger.h>
#include <chrono>

using namespace FastQ;
using namespace FastQ::SampleIdl;

static const LogModule LM_APP {"FASTQ_TEST_APP"};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test consumer app");

	FastQ::Consumer<128, 100000> consumer {"/dev/shm/test.shm"};
	consumer.Start();

	std::chrono::seconds duration(60);
	auto start = std::chrono::steady_clock::now();

	LOG(INFO, LM_APP, "reading data..");

	SampleData data {};
	uint64_t readCount = 0;
	while (std::chrono::steady_clock::now() - start < duration)
	{
		// sync up
		const auto result = consumer.Pop(&data);
		if (!result)
			continue;

		if (readCount == 440617601)
			break;

		if (readCount == 0)
		{
			readCount = data.mId;
		}
		else
		{
			THROW_IF(data.mId != readCount, "missed a packet, packet count: " + readCount)
		}
		LOG(INFO, LM_APP, "read: " << readCount);
		readCount++;
	};

	LOG(INFO, LM_APP, "total read count: " << readCount);

	return 1;
}