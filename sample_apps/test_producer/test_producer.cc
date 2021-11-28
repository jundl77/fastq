#include <producer/producer.h>
#include <sample_idl/sample_idl.h>
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
	LOG(INFO, LM_APP, "producer shutting down clean");
	globalProducer->Shutdown();
	LOG(INFO, LM_APP, "clean shutdown success");
	std::exit(1);
}

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

	LOG(INFO, LM_APP, "starting test producer app, duration: %d sec, logging_enabled: %d",
		duration.count(), shouldLog);

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

	LOG(INFO, LM_APP, "writing data..");

	auto start = std::chrono::steady_clock::now();

	uint64_t writeCount = 0;
	while (std::chrono::steady_clock::now() - start < duration)
	{
		data.mId = writeCount;
		producer.Push(1, &data, sizeof(data));
		writeCount++;
		if (shouldLog) { LOG(INFO, LM_APP, "wrote: %llu", writeCount); }
		//std::this_thread::sleep_for(1ms);
	}

	auto realDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
	LOG(INFO, LM_APP, "total write count: %llu over %d ms", writeCount, realDuration.count());
	double mbPerSec = (writeCount * sizeof(data) * 1.0) / (1024.0 * 1024.0) / duration.count();
	LOG(INFO, LM_APP, "[write_metric] {\"mb_per_sec\": %f, \"finished\": 1}", mbPerSec);

	return 1;
}