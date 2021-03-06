#include <producer/producer.h>
#include <sample_idl/sample_idl.h>
#include <core/tsc_clock.h>
#include <core/logger.h>
#include <chrono>
#include <thread>
#include <cstdint>
#include <csignal>

using namespace FastQ;
using namespace FastQ::SampleIdl;
using namespace std::chrono_literals;

static const LogModule LM_APP {"FASTQ_TEST_APP"};

Producer* globalProducer;

void Shutdown()
{
	LOG(FASTQ_INFO, LM_APP, "producer shutting down clean");
	globalProducer->Shutdown();
	LOG(FASTQ_INFO, LM_APP, "clean shutdown success");
	std::exit(1);
}

int main(int argc, const char** argv)
{
	FastQ::SetGlobalLogLevel(FASTQ_INFO);

	if (argc < 2 || argc > 4)
	{
		std::cout << "Usage: " << argv[0] << " [run_duration] \"--log\" (optional), \"--slow\" (optional)"<< std::endl;
		return 1;
	}

	std::chrono::seconds duration = std::chrono::seconds(std::atoi(argv[1]));

	bool shouldLog = false;
	bool goSlow = false;

	int i = 2;
	while (i < argc)
	{
		const std::string arg {argv[i]};
		if (arg == "--log")
		{
			shouldLog = true;
		}
		if (arg == "--slow")
		{
			goSlow = true;
		}
		i += 1;
	}

	LOG(FASTQ_INFO, LM_APP, "starting test producer app, duration: %d sec, logging_enabled: %d, go_slow: %d",
		duration.count(), shouldLog, goSlow);

	int size = 1024 * 1024 * 1024;
	Producer producer {"test_shm", size};
	globalProducer = &producer;

	std::signal(SIGINT, [](int signal) { Shutdown(); });
	std::signal(SIGABRT, [](int signal) { Shutdown(); });
	std::signal(SIGTERM, [](int signal) { Shutdown(); });

	producer.Start();

	SampleData data;
	for (int i = 0; i < sizeof(data.mData); i++)
	{
		data.mData[i] = i;
	}

	LOG(FASTQ_INFO, LM_APP, "writing data..");

	TSCClock::Initialise();
	const uint64_t durationInCycles = TSCClock::ToCycles<std::chrono::seconds>(duration);
	const uint64_t start = TSCClock::NowInCycles();

	uint64_t writeCount = 0;
	bool isRunning = true;
	while (isRunning)
	{
		if (writeCount % 10000)
		{
			isRunning = TSCClock::NowInCycles() - start < durationInCycles;
		}
		data.mId = writeCount;
		producer.Push(1, &data, sizeof(data));
		writeCount++;
		if (shouldLog) { LOG(FASTQ_INFO, LM_APP, "wrote: %llu", writeCount); }
		if (goSlow) { std::this_thread::sleep_for(1ms); }
	}

	auto realDuration = std::chrono::duration_cast<std::chrono::milliseconds>(TSCClock::Now() - TSCClock::FromCycles(start));
	LOG(FASTQ_INFO, LM_APP, "total write count: %llu over %d ms", writeCount, realDuration.count());
	double mbPerSec = (writeCount * sizeof(data) * 1.0) / (1024.0 * 1024.0) / duration.count();
	LOG(FASTQ_INFO, LM_APP, "[write_metric] {\"mb_per_sec\": %f, \"finished\": 1}", mbPerSec);

	return 1;
}