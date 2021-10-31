#include <producer/producer.h>
#include <consumer/consumer.h>
#include <core/logger.h>

static const LogModule LM_APP {"FASTQ_TEST_APP"};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test app");

	FastQ::Producer producer {"test.shm", 2};
	producer.Start();

	FastQ::Consumer consumer {"test.shm", 2};
	consumer.Start();

	return 1;
}