#include "types.h"

#include <producer/producer.h>
#include <consumer/consumer.h>
#include <gtest/gtest.h>

namespace FastQ::Testing {

static const std::string SHM_FILE {"test.shm"};

class PerformanceTests : public ::testing::Test
{
public:
	PerformanceTests()
	{}

protected:
	void Start()
	{

	}

protected:

};

TEST_F(PerformanceTests, HeadersMatch)
{
	Start();
}

}