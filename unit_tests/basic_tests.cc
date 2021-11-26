#include "types.h"

#include <producer/producer.h>
#include <consumer/consumer.h>
#include <gtest/gtest.h>

namespace FastQ::Testing {

static const std::string SHM_FILE {"test.shm"};

struct QueueElement
{
	u_int32_t mType {0};
	u_int32_t mSize {0};
	std::vector<uint8_t> mData;
};

class BasicTests : public IFastQHandler, public ::testing::Test
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
		CompareHeaders((void*)&mProducer.GetHeader(), (void*)&mConsumer1.GetHeader());
		CompareHeaders((void*)&mProducer.GetHeader(), (void*)&mConsumer2.GetHeader());
		CompareHeaders((void*)&mProducer.GetHeader(), (void*)&mConsumer3.GetHeader());
	}

	void OnData(u_int32_t type, void* data, u_int32_t size) override
	{
		mLastElement = {};
		mLastElement.mData.reserve(size);
		mLastElement.mType = type;
		mLastElement.mSize = size;
		std::memcpy(mLastElement.mData.data(), data, size);
	}

	void CompareHeaders(void* header1, void* header2)
	{
		const auto* h1 = (const Idl::Header*) header1;
		const auto* h2 = (const Idl::Header*) header2;
		EXPECT_EQ(std::string(h1->mProtocolName), std::string(h2->mProtocolName));
		EXPECT_EQ(h1->mVersionMajor, h2->mVersionMajor);
		EXPECT_EQ(h1->mVersionMinor, h2->mVersionMinor);
		EXPECT_EQ(h1->mMagicNumber, h2->mMagicNumber);
		EXPECT_EQ(h1->mFileSize, h2->mFileSize);
		EXPECT_EQ(h1->mPayloadSize, h2->mPayloadSize);
	}

	void CompareWithLastElement(const SimpleFoo& foo)
	{
		EXPECT_EQ(foo.mType, mLastElement.mType);
		EXPECT_EQ(sizeof(foo), mLastElement.mSize);
		CompareSimpleFoo(foo, reinterpret_cast<SimpleFoo&>(*mLastElement.mData.data()));
		mLastElement = {};  // reset once read
	}

	void CompareSimpleFoo(const SimpleFoo& foo1, const SimpleFoo& foo2)
	{
		EXPECT_EQ(foo1.mType, foo2.mType);
		EXPECT_EQ(foo1.mFoo1, foo2.mFoo1);
		EXPECT_EQ(foo1.mFoo2, foo2.mFoo2);
		EXPECT_EQ(foo1.mFoo3, foo2.mFoo3);
	}

protected:
	FastQ::Producer mProducer{SHM_FILE, 1024 * 2};
	FastQ::Consumer mConsumer1{SHM_FILE, *this};
	FastQ::Consumer mConsumer2{SHM_FILE, *this};
	FastQ::Consumer mConsumer3{SHM_FILE, *this};
	QueueElement mLastElement;

	SimpleFoo mFoo1 {
		.mType = 80,
		.mFoo1 = 1,
		.mFoo2 = 2,
		.mFoo3 = 3
	};
	SimpleFoo mFoo2 {
		.mType = 90,
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

	mProducer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo1));

	EXPECT_TRUE(mConsumer1.Poll());
	CompareWithLastElement(mFoo1);

	EXPECT_TRUE(mConsumer2.Poll());
	CompareWithLastElement(mFoo1);

	EXPECT_TRUE(mConsumer3.Poll());
	CompareWithLastElement(mFoo1);
}

TEST_F(BasicTests, MultiplePushPop)
{
	Start();

	mProducer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo2));
	mProducer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo1));
	mProducer.Push(mFoo2.mType,&mFoo2, sizeof(mFoo2));

	EXPECT_TRUE(mConsumer1.Poll());
	CompareWithLastElement(mFoo1);
	EXPECT_TRUE(mConsumer1.Poll());
	CompareWithLastElement(mFoo1);
	EXPECT_TRUE(mConsumer1.Poll());
	CompareWithLastElement(mFoo2);
	EXPECT_FALSE(mConsumer1.Poll());

	EXPECT_TRUE(mConsumer2.Poll());
	CompareWithLastElement(mFoo1);
	EXPECT_TRUE(mConsumer2.Poll());
	CompareWithLastElement(mFoo1);
	EXPECT_TRUE(mConsumer2.Poll());
	CompareWithLastElement(mFoo2);
	EXPECT_FALSE(mConsumer2.Poll());

	EXPECT_TRUE(mConsumer3.Poll());
	CompareWithLastElement(mFoo1);
	EXPECT_TRUE(mConsumer3.Poll());
	CompareWithLastElement(mFoo1);
	EXPECT_TRUE(mConsumer3.Poll());
	CompareWithLastElement(mFoo2);
	EXPECT_FALSE(mConsumer3.Poll());
}

TEST_F(BasicTests, WrapAround)
{
	FastQ::Producer producer{SHM_FILE, sizeof(SimpleFoo) * 2};
	FastQ::Consumer consumer{SHM_FILE, *this};

	producer.Start();
	consumer.Start();

	producer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo1));
	EXPECT_TRUE(consumer.Poll());
	CompareWithLastElement(mFoo1);
	producer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo1));
	EXPECT_TRUE(consumer.Poll());
	CompareWithLastElement(mFoo1);
	producer.Push(mFoo2.mType, &mFoo2, sizeof(mFoo2));
	EXPECT_TRUE(consumer.Poll());
	CompareWithLastElement(mFoo2);

	EXPECT_FALSE(consumer.Poll());
}

TEST_F(BasicTests, WrapAround_PayloadOnly)
{
	const size_t filesize = Idl::FASTQ_SIZE_WITHOUT_PAYLOAD + sizeof(SimpleFoo) + Idl::FASTQ_FRAMING_HEADER_SIZE * 2;
	FastQ::Producer producer{SHM_FILE, filesize};
	FastQ::Consumer consumer{SHM_FILE, *this};

	producer.Start();
	consumer.Start();

	producer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo1));
	EXPECT_TRUE(consumer.Poll());
	CompareWithLastElement(mFoo1);
	producer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo1));
	EXPECT_TRUE(consumer.Poll());
	CompareWithLastElement(mFoo1);
	producer.Push(mFoo2.mType, &mFoo2, sizeof(mFoo2));
	EXPECT_TRUE(consumer.Poll());
	CompareWithLastElement(mFoo2);

	EXPECT_FALSE(consumer.Poll());
}

TEST_F(BasicTests, SingleProducerSingleConsumer_TestProducerOvertakesConsumer)
{
//	Start();
//
//	for (int i = 0; i < 100; i++)
//	{
//		mProducer.Push(mFoo1.mType, &mFoo1, sizeof(mFoo1));
//	}
//	EXPECT_THROW(mConsumer1.Poll(), std::runtime_error);
//	EXPECT_THROW(mConsumer3.Poll(), std::runtime_error);
//	EXPECT_THROW(mConsumer2.Poll(), std::runtime_error);
}

}
