#include <consumer/consumer.h>
#include <sample_idl/sample_idl.h>
#include <core/logger.h>
#include <chrono>

using namespace FastQ;
using namespace FastQ::SampleIdl;

static const LogModule LM_APP {"FASTQ_TEST_APP"};

class Handler : public IFastQHandler
{
public:
	void OnData(u_int32_t type, void* data, u_int32_t size)
	{
		auto* sampleData = (SampleData*) data;
		if (mReadCount == 0)
		{
			mReadCount = sampleData->mId;
		}
		else
		{
			THROW_IF(sampleData->mId != mReadCount, "missed a packet, packet count: %d, but last encountered was: %d",
				mReadCount, sampleData->mId)
		}
		LOG(INFO, LM_APP, "read: %d", mReadCount);
		mReadCount++;
	}

	u_int32_t mReadCount {0};
};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test consumer app");

	Handler handler;
	FastQ::Consumer consumer {"test.shm", handler};
	consumer.Start();

	std::chrono::seconds duration(60);
	auto start = std::chrono::steady_clock::now();

	LOG(INFO, LM_APP, "reading data..");

	while (std::chrono::steady_clock::now() - start < duration)
	{
		consumer.Poll();
	};

	LOG(INFO, LM_APP, "total read count: %d", handler.mReadCount);
	return 1;
}