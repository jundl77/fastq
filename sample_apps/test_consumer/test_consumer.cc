#include <consumer/consumer.h>
#include <core/logger.h>

static const LogModule LM_APP {"FASTQ_TEST_APP"};

int main(int argc, const char** argv)
{
	LOG(INFO, LM_APP, "starting test consumer app");

	FastQ::Consumer<10, 100> consumer {"../test.shm"};
	consumer.Start();

	//while (true) { };
	return 1;
}