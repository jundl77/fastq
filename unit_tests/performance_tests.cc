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
		mProducer.Start();
		mConsumer1.Start();
		mConsumer2.Start();
		mConsumer3.Start();
	}

protected:
	FastQ::Producer<sizeof(SimpleFoo)> mProducer{SHM_FILE, 2048};
	FastQ::Consumer<sizeof(SimpleFoo)> mConsumer1{SHM_FILE};
	FastQ::Consumer<sizeof(SimpleFoo)> mConsumer2{SHM_FILE};
	FastQ::Consumer<sizeof(SimpleFoo)> mConsumer3{SHM_FILE};

	SimpleFoo mFoo1 {
		.mFoo1 = 1,
		.mFoo2 = 2,
		.mFoo3 = 3
	};
	SimpleFoo mFoo2 {
		.mFoo1 = 4,
		.mFoo2 = 5,
		.mFoo3 = 6
	};
};

TEST_F(PerformanceTests, HeadersMatch)
{
	Start();
}

}