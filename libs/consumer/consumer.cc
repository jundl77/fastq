#include "consumer.h"

namespace FastQ {

static const LogModule LM_CONSUMER {"FASTQ_CONSUMER"};

Consumer::Consumer(std::string shmFilename, IFastQHandler& handler)
	: FastQCore(LM_CONSUMER)
	, mShmFilename(std::move(shmFilename))
	, mHandler(handler)
{
	mCurrentReadBuffer.reserve(1024 * 1024 * 10); // 10mb
}

void Consumer::Start()
{
	THROW_IF(mConnected, "fastq was already started, it can only be started once")
	LOG(FASTQ_INFO, LM_CONSUMER, "starting consumer")
	mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);

	// read header for size
	mFastQBuffer->Mmap(sizeof(Idl::Header), MmapProtMode::READ_ONLY);
	mFastQueue = reinterpret_cast<Idl::FastQueue*>(mFastQBuffer->GetAddress());
	ValidateHeader();
	mFileSize = mFastQueue->mHeader.mFileSize;
	mPayloadSize = mFastQueue->mHeader.mPayloadSize;
	mMagicNumber = mFastQueue->mHeader.mMagicNumber;

	// re-map entire queue knowing size
	mFastQBuffer->Munmap();
	mFastQBuffer->Mmap(mFileSize, MmapProtMode::READ_ONLY);
	mFastQueue = reinterpret_cast<Idl::FastQueue*>(mFastQBuffer->GetAddress());

	FastQCore::Init(mFastQueue);
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo.load();
	mWrapAroundCounter = GetWrapAroundCount(lastWriteInfo);
	mLastReadPosition = GetLastWritePosition(lastWriteInfo);
	LOG(FASTQ_INFO, LM_CONSUMER, "position in queue:")
	LOG(FASTQ_INFO, LM_CONSUMER, "  wrap around count:  %d", mWrapAroundCounter)
	LOG(FASTQ_INFO, LM_CONSUMER, "  last read position: %d", mLastReadPosition)

	mConnected = true;
	mHandler.OnConnected();
}

void Consumer::Shutdown()
{
	mFastQBuffer->Close();
	mConnected = false;
}

bool Consumer::Poll()
{
	DEBUG_THROW_IF(!mConnected, "Poll was called, but fastq is not connected");
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo.load();
	const uint32_t queueWrapAround = GetWrapAroundCount(lastWriteInfo);
	const uint32_t queueLastWritePosition = GetLastWritePosition(lastWriteInfo);
	if (mLastReadPosition == queueLastWritePosition && mWrapAroundCounter == queueWrapAround)
	{
		// no new elements in queue, we are caught up
		return false;
	}
	DEBUG_THROW_IF(mWrapAroundCounter > queueWrapAround, "wrap count of reader is ahead of that of writer, is there a race condition?");
	DEBUG_THROW_IF(mLastReadPosition > queueLastWritePosition && mWrapAroundCounter >= queueWrapAround,
				"reader is ahead of writer, is there a race condition?");

	// check to make sure we were not overtaken by the producer
	if (!AssertInSync()) { return false; }
	Idl::FramingHeader framingHeader;
	mLastReadPosition = ReadData(mLastReadPosition, &framingHeader, sizeof(Idl::FramingHeader));
  
	// check to make sure we were not overtaken while reading the header
	if (!AssertInSync()) { return false; }
	if (mCurrentReadBuffer.capacity() < framingHeader.mSize)
	{
		mCurrentReadBuffer.reserve(framingHeader.mSize);
	}
	DEBUG_THROW_IF(mCurrentReadBuffer.capacity() < framingHeader.mSize, "read buffer is too small");
	mLastReadPosition = ReadData(mLastReadPosition, mCurrentReadBuffer.data(), framingHeader.mSize);

	// check after copying the data into the read buffer to make sure we have not been overtaken by the producer
	if (!AssertInSync()) { return false; }
	mHandler.OnData(framingHeader.mType, mCurrentReadBuffer.data(), framingHeader.mSize);
	return true;
}

uint32_t Consumer::ReadData(uint32_t lastReadPosition, void* data, uint32_t size)
{
	uint8_t* payload = GetPayloadPointer();
	uint32_t endReadPosition = FastQCore::NextFramePosition(lastReadPosition, size);

	// if we have wrapped around, set writeAddr to the start of the payload again, otherwise keep going
	uint8_t* readAddr = payload + (endReadPosition - size);
	if (endReadPosition == 0)
	{
		mWrapAroundCounter++;
		readAddr = payload;
		endReadPosition = size;
		DEBUG_LOG(FASTQ_INFO, LM_CONSUMER, "consumer wrapped around, wrap-around counter: %d", mWrapAroundCounter)
	}

	std::memcpy(data, readAddr, size);
	DEBUG_LOG(FASTQ_INFO, LM_CONSUMER, "[consumer state] last read position %d, wrap-around counter: %d",
			  endReadPosition, mWrapAroundCounter)
	return endReadPosition;
}

bool Consumer::AssertInSync()
{
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo.load();
	const uint32_t queueWrapAround = GetWrapAroundCount(lastWriteInfo);
	const uint32_t queueLastWritePosition = GetLastWritePosition(lastWriteInfo);

	DEBUG_THROW_IF(mWrapAroundCounter > queueWrapAround, "wrap count of reader is ahead of that of writer, is there a race condition?");
	DEBUG_THROW_IF(mLastReadPosition > queueLastWritePosition && mWrapAroundCounter >= queueWrapAround,
				   "reader is ahead of writer, is there a race condition?");
	const uint32_t sizeBehind = (queueWrapAround - mWrapAroundCounter) * mPayloadSize + (queueLastWritePosition - mLastReadPosition);

	if (sizeBehind > mPayloadSize)
	{
		Shutdown();
		mHandler.OnDisconnected("reader was too slow, writer looped around and overwrote reader", DisconnectType::DISCONNECT_WITH_ERROR);
	}
	return mConnected;
}

void Consumer::ValidateHeader()
{
	THROW_IF(mFastQueue->mHeader.mVersionMajor != Idl::FASTQ_MAJOR_VERSION,
			 "FastQueue protocol major version mismatch, producer has version %d, but consumer has version %d",
			 mFastQueue->mHeader.mVersionMajor, Idl::FASTQ_MAJOR_VERSION);
	THROW_IF(mFastQueue->mHeader.mVersionMinor < Idl::FASTQ_MAJOR_VERSION,
			 "FastQueue protocol minor version mismatch, producer version is older than consumer, producer version: %d, consumer version: %d",
			 mFastQueue->mHeader.mVersionMinor, Idl::FASTQ_MAJOR_VERSION);
	if (mMagicNumber != 0)
	{
		THROW_IF(mFastQueue->mHeader.mMagicNumber != mMagicNumber,
				"magic numbers between producer and consumer dont' match, was the producer restarted?");
	}
}

}