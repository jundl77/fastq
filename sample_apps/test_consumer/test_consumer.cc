#include <consumer/consumer.h>
#include <sample_idl/sample_idl.h>
#include <shared_lib/statistics.h>
#include <core/tsc_clock.h>
#include <core/logger.h>
#include <chrono>
#include <csignal>

using namespace FastQ;
using namespace FastQ::SampleIdl;
using namespace FastQ::SampleApps;

static const LogModule LM_APP {"FASTQ_TEST_APP"};

Consumer* globalConsumer;

void Shutdown()
{
	LOG(FASTQ_INFO, LM_APP, "consumer shutting down clean");
	globalConsumer->Shutdown();
	LOG(FASTQ_INFO, LM_APP, "clean shutdown success");
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
		LOG(FASTQ_INFO, LM_APP, "consumer is connected");
	}

	void OnDisconnected(const std::string& reason, DisconnectType) override
	{
		LOG(FASTQ_ERROR, LM_APP, "consumer disconnected with reason: %s", reason.c_str());
		mExitedOnError = true;
	}

	void OnData(u_int32_t type, void* data, u_int32_t size) override
	{
		const SampleData* sampleData = (SampleData*) data;
		if (mReadCount == 0)
		{
			mReadCount = sampleData->mId;
		}
		else
		{
			THROW_IF(sampleData->mId != mReadCount, "missed a packet, packet count: %d, but last encountered was: %d",
				mReadCount, sampleData->mId)
		}
		if (mShouldLog) { LOG(FASTQ_INFO, LM_APP, "read: %d", mReadCount); }
		mReadCount++;
	}

	bool mShouldLog {true};
	uint64_t mReadCount {0};
	bool mExitedOnError {false};
};

int main(int argc, const char** argv)
{
	FastQ::SetGlobalLogLevel(FASTQ_DEBUG);
	if (argc < 2 || argc > 3)
	{
		std::cout << "Usage: " << argv[0] << " [run_duration] \"--log\" (optional) "<< std::endl;
		return 1;
	}

	std::chrono::seconds duration = std::chrono::seconds(std::atoi(argv[1]));

	bool shouldLog = false;
	if (argc == 3 && std::string(argv[2]) == "--log")
	{
		shouldLog = true;
	}

	LOG(FASTQ_INFO, LM_APP, "starting test consumer app, duration: %d sec, logging_enabled: %d",
		duration.count(), shouldLog);

	Handler handler {shouldLog};
	FastQ::Consumer consumer {"test_shm", handler};
	globalConsumer = &consumer;

	std::signal(SIGINT, [](int signal) { Shutdown(); });
	std::signal(SIGABRT, [](int signal) { Shutdown(); });
	std::signal(SIGTERM, [](int signal) { Shutdown(); });

	TSCClock::Initialise();
	const uint64_t durationInCycles = TSCClock::ToCycles<std::chrono::seconds>(duration);
	const uint64_t start = TSCClock::NowInCycles();
	StatisticsCollector<uint64_t> statistics(std::chrono::nanoseconds::max().count(), 0);

	consumer.Start();
	LOG(FASTQ_INFO, LM_APP, "reading data..");
	THROW_IF(!consumer.IsConnected(), "consumer not connected to producer, is producer running?");
	try
	{
		while (consumer.IsConnected())
		{
			uint64_t pollStartTs = TSCClock::NowInCycles();
			if (pollStartTs - start > durationInCycles) { break; }
			consumer.Poll();
			std::chrono::nanoseconds pollDurationNs = TSCClock::FromCycles(TSCClock::NowInCycles() - pollStartTs);
			statistics.Record(pollDurationNs.count());
			if (pollDurationNs.count() > 10000)
			{
				LOG(FASTQ_WARN, LM_APP, "poll duration over 10us: %d ns, cycle_count: %llu", pollDurationNs, statistics.GetCount());
			}
		}
	}
	catch (const std::runtime_error& error)
	{
		LOG(FASTQ_ERROR, LM_APP, "consumer crashed with error: %s", error.what());
	}

	const std::chrono::milliseconds realDuration =
			std::chrono::duration_cast<std::chrono::milliseconds>(TSCClock::Now() - TSCClock::FromCycles(start));
	LOG(FASTQ_INFO, LM_APP, "total read count: %llu over %d ms", handler.mReadCount, realDuration.count());
	const double mbPerSec = (handler.mReadCount * sizeof(SampleData) * 1.0) / (1024.0 * 1024.0) / duration.count();
	LOG(FASTQ_INFO, LM_APP, "[read_metric] {\"mb_per_sec\": %f, \"finished\": %d}", mbPerSec, !handler.mExitedOnError);
	statistics.Report(LM_APP, "read_metric", "poll_duration_ns");
	return 1;
}