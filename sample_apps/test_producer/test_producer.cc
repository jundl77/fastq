#include <producer/producer.h>

int main(int argc, const char** argv)
{
	FastQ::Producer producer {};
	producer.Produce();
	return 1;
}