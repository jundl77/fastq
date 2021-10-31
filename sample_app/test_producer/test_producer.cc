#include <producer/producer.h>
#include <core/logger.h>

static const LogModule LM_APP {"FASTQ_TEST_APP"};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test producer app");

	FastQ::Producer producer {"../test.shm", 2};
	producer.Start();

	while (true) { };
	return 1;
}