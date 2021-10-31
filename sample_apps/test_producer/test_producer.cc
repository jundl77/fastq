#include <producer/producer.h>
#include <core/logger.h>

static const LogModule LM_APP {"FASTQ_TEST_APP"};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test producer app");

	FastQ::Producer<10, 100> producer {"../test.shm", 10 * 100};
	producer.Start();

	while (true)
	{
		producer.Poll_100ms();
	};

	return 1;
}