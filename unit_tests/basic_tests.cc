#include "types.h"

#include <producer/producer.h>
#include <consumer/consumer.h>
#include <gtest/gtest.h>

namespace FastQ::Testing {

static const std::string SHM_FILE {"test.shm"};

class BasicTests : public ::testing::Test
{
public:
	BasicTests()
	{}

protected:
	void Start()
	{
		mProducer.Start();
		mConsumer1.Start();
		mConsumer2.Start();
		mConsumer3.Start();
		CompareHeaders<sizeof(SimpleFoo)>((void*)&mProducer.GetHeader(), (void*)&mConsumer1.GetHeader());
		CompareHeaders<sizeof(SimpleFoo)>((void*)&mProducer.GetHeader(), (void*)&mConsumer2.GetHeader());
		CompareHeaders<sizeof(SimpleFoo)>((void*)&mProducer.GetHeader(), (void*)&mConsumer3.GetHeader());
	}

	template<uint32_t FrameSizeT>
	void CompareHeaders(void* header1, void* header2)
	{
		const auto* h1 = (const Idl::Header<FrameSizeT>*) header1;
		const auto* h2 = (const Idl::Header<FrameSizeT>*) header2;
		EXPECT_EQ(std::string(h1->mProtocolName), std::string(h2->mProtocolName));
		EXPECT_EQ(h1->mVersionMajor, h2->mVersionMajor);
		EXPECT_EQ(h1->mVersionMinor, h2->mVersionMinor);
		EXPECT_EQ(h1->mMagicNumber, h2->mMagicNumber);
		EXPECT_EQ(h1->mFrameSize, h2->mFrameSize);
		EXPECT_EQ(h1->mMaxFrameCount, h2->mMaxFrameCount);
		EXPECT_EQ(h1->mPayloadSize, h2->mPayloadSize);
	}

	void CompareSimpleFoo(const SimpleFoo& foo1, const SimpleFoo& foo2)
	{
		EXPECT_EQ(foo1.mFoo1, foo2.mFoo1);
		EXPECT_EQ(foo1.mFoo2, foo2.mFoo2);
		EXPECT_EQ(foo1.mFoo3, foo2.mFoo3);
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

TEST_F(BasicTests, HeadersMatch)
{
	Start();
}

TEST_F(BasicTests, SinglePushPop)
{
	Start();

	mProducer.Push(&mFoo1, sizeof(mFoo1));

	SimpleFoo foo1 {};
	EXPECT_TRUE(mConsumer1.Pop(&foo1));
	CompareSimpleFoo(mFoo1, foo1);

	SimpleFoo foo2 {};
	EXPECT_TRUE(mConsumer2.Pop(&foo2));
	CompareSimpleFoo(mFoo1, foo2);

	SimpleFoo foo3 {};
	EXPECT_TRUE(mConsumer3.Pop(&foo3));
	CompareSimpleFoo(mFoo1, foo3);
}

TEST_F(BasicTests, MultiplePushPop)
{
	Start();

	mProducer.Push(&mFoo1, sizeof(mFoo2));
	mProducer.Push(&mFoo1, sizeof(mFoo2));
	mProducer.Push(&mFoo2, sizeof(mFoo2));

	SimpleFoo foo {};
	EXPECT_TRUE(mConsumer1.Pop(&foo));
	CompareSimpleFoo(mFoo1, foo);
	EXPECT_TRUE(mConsumer1.Pop(&foo));
	CompareSimpleFoo(mFoo1, foo);
	EXPECT_TRUE(mConsumer1.Pop(&foo));
	CompareSimpleFoo(mFoo2, foo);
	EXPECT_FALSE(mConsumer1.Pop(&foo));

	EXPECT_TRUE(mConsumer2.Pop(&foo));
	CompareSimpleFoo(mFoo1, foo);
	EXPECT_TRUE(mConsumer2.Pop(&foo));
	CompareSimpleFoo(mFoo1, foo);
	EXPECT_TRUE(mConsumer2.Pop(&foo));
	CompareSimpleFoo(mFoo2, foo);
	EXPECT_FALSE(mConsumer2.Pop(&foo));

	EXPECT_TRUE(mConsumer3.Pop(&foo));
	CompareSimpleFoo(mFoo1, foo);
	EXPECT_TRUE(mConsumer3.Pop(&foo));
	CompareSimpleFoo(mFoo1, foo);
	EXPECT_TRUE(mConsumer3.Pop(&foo));
	CompareSimpleFoo(mFoo2, foo);
	EXPECT_FALSE(mConsumer3.Pop(&foo));
}

TEST_F(BasicTests, WrapAround)
{
	FastQ::Producer<sizeof(SimpleFoo)> producer{SHM_FILE, sizeof(SimpleFoo) * 2};
	FastQ::Consumer<sizeof(SimpleFoo)> consumer{SHM_FILE};

	producer.Start();
	consumer.Start();
	SimpleFoo foo {};

	producer.Push(&mFoo1, sizeof(mFoo1));
	EXPECT_TRUE(consumer.Pop(&foo));
	producer.Push(&mFoo1, sizeof(mFoo1));
	EXPECT_TRUE(consumer.Pop(&foo));
	producer.Push(&mFoo2, sizeof(mFoo2));
	EXPECT_TRUE(consumer.Pop(&foo));
	EXPECT_FALSE(consumer.Pop(&foo));

	CompareSimpleFoo(mFoo2, foo);
}

TEST_F(BasicTests, SingleProducerSingleConsumer_TestFallBehindLimit)
{
	Start();
	SimpleFoo foo {};

	for (int i = 0; i < 100; i++)
	{
		mProducer.Push(&mFoo1, sizeof(mFoo1));
	}
	EXPECT_THROW(mConsumer1.Pop(&foo), std::runtime_error);
	EXPECT_THROW(mConsumer3.Pop(&foo), std::runtime_error);
	EXPECT_THROW(mConsumer2.Pop(&foo), std::runtime_error);
}

}
