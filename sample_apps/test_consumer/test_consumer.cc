#include <consumer/consumer.h>
#include <sample_idl/sample_idl.h>
#include <core/logger.h>
#include <chrono>
#include <csignal>

using namespace FastQ;
using namespace FastQ::SampleIdl;

static const LogModule LM_APP {"FASTQ_TEST_APP"};

Consumer* globalConsumer;

void Shutdown()
{
	LOG(INFO, LM_APP, "consumer shutting down clean");
	globalConsumer->Shutdown();
	LOG(INFO, LM_APP, "clean shutdown success");
	std::exit(1);
}

class Handler : public IFastQHandler
{
public:
	explicit Handler(bool shouldLog)
		: mShouldLog(shouldLog)
	{}

	void OnConnected() override
	{
		LOG(INFO, LM_APP, "consumer is connected");
	}

	void OnDisconnected(const std::string& reason, DisconnectType) override
	{
		LOG(ERROR, LM_APP, "consumer disconnected with reason: %s", reason.c_str());
		mExitedOnError = true;
	}

	void OnData(u_int32_t type, void* data, u_int32_t size) override
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
		if (mShouldLog) { LOG(INFO, LM_APP, "read: %d", mReadCount); }
		mReadCount++;
	}

	bool mShouldLog {true};
	uint64_t mReadCount {0};
	bool mExitedOnError {false};
};

int main(int argc, const char** argv)
{
	FastQ::SetGlobalLogLevel(DEBUG);
	if (argc < 2 || argc > 3)
	{
		std::cout << "Usage: " << argv[0] << " [run_duration] \"no_log\" (optional) "<< std::endl;
		return 1;
	}

	std::chrono::seconds duration = std::chrono::seconds(std::atoi(argv[1]));

	bool shouldLog = true;
	if (argc == 3 && std::string(argv[2]) == "no_log")
	{
		shouldLog = false;
	}

	LOG(INFO, LM_APP, "starting test consumer app, duration: %d sec, logging_enabled: %d",
		duration.count(), shouldLog);

	Handler handler {shouldLog};
	FastQ::Consumer consumer {"test_shm", handler};
	globalConsumer = &consumer;

	std::signal(SIGINT, [](int signal) { Shutdown(); });
	std::signal(SIGABRT, [](int signal) { Shutdown(); });
	std::signal(SIGTERM, [](int signal) { Shutdown(); });

	consumer.Start();
	LOG(INFO, LM_APP, "reading data..");

	auto start = std::chrono::steady_clock::now();
	while (consumer.IsConnected() && std::chrono::steady_clock::now() - start < duration)
	{
		consumer.Poll();
	}

	auto realDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
	LOG(INFO, LM_APP, "total read count: %llu over %d ms", handler.mReadCount, realDuration.count());
	double mbPerSec = (handler.mReadCount * sizeof(SampleData) * 1.0) / (1024.0 * 1024.0) / duration.count();
	LOG(INFO, LM_APP, "[read_metric] {\"mb_per_sec\": %f, \"finished\": %d}", mbPerSec, !handler.mExitedOnError);
	return 1;
}