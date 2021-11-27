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
	LOG(INFO, LM_CONSUMER, "starting consumer")
	mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);

	// read header for size
	mFastQBuffer->Mmap(Idl::FASTQ_HEADER_SIZE, MmapProtMode::READ_ONLY);
	mFastQueue = reinterpret_cast<Idl::FastQueue*>(mFastQBuffer->GetAddress());
	ValidateHeader();
	mFileSize = mFastQueue->mHeader.mFileSize;
	mPayloadSize = mFastQueue->mHeader.mPayloadSize;

	// re-map entire queue knowing size
	mFastQBuffer->Munmap();
	mFastQBuffer->Mmap(mFileSize, MmapProtMode::READ_ONLY);
	mFastQueue = reinterpret_cast<Idl::FastQueue*>(mFastQBuffer->GetAddress());

	FastQCore::Init(mFastQueue);
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo.load();
	mWrapAroundCounter = GetWrapAroundCount(lastWriteInfo);
	mLastReadPosition = GetLastWritePosition(lastWriteInfo);
	LOG(INFO, LM_CONSUMER, "position in queue:")
	LOG(INFO, LM_CONSUMER, "  wrap around count:  %d", mWrapAroundCounter)
	LOG(INFO, LM_CONSUMER, "  last read position: %d", mLastReadPosition)

	mConnected = true;
	mHandler.OnConnected();
}

void Consumer::Shutdown()
{
	THROW_IF(!mConnected, "fastq cannot be shutdown, it is not running")
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
	AssertInSync();
	Idl::FramingHeader framingHeader;
	mLastReadPosition = ReadData(mLastReadPosition, &framingHeader, Idl::FASTQ_FRAMING_HEADER_SIZE);
  
	// check to make sure we were not overtaken while reading the header
	AssertInSync();
	if (mCurrentReadBuffer.capacity() < framingHeader.mSize)
	{
		mCurrentReadBuffer.reserve(framingHeader.mSize);
	}
	DEBUG_THROW_IF(mCurrentReadBuffer.capacity() < framingHeader.mSize, "read buffer is too small");
	mLastReadPosition = ReadData(mLastReadPosition, mCurrentReadBuffer.data(), framingHeader.mSize);

	// check after copying the data into the read buffer to make sure we have not been overtaken by the producer
	AssertInSync();
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
		DEBUG_LOG(INFO, LM_CONSUMER, "consumer wrapped around, wrap-around counter: %d", mWrapAroundCounter)
	}

	std::memcpy(data, readAddr, size);
	DEBUG_LOG(INFO, LM_CONSUMER, "[consumer state] last read position %d, wrap-around counter: %d",
			  endReadPosition, mWrapAroundCounter)
	return endReadPosition;
}

void Consumer::AssertInSync()
{
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo.load();
	const uint32_t queueWrapAround = GetWrapAroundCount(lastWriteInfo);
	const uint32_t queueLastWritePosition = GetLastWritePosition(lastWriteInfo);

	DEBUG_THROW_IF(mWrapAroundCounter > queueWrapAround, "wrap count of reader is ahead of that of writer, is there a race condition?");
	DEBUG_THROW_IF(mLastReadPosition > queueLastWritePosition && mWrapAroundCounter >= queueWrapAround,
				   "reader is ahead of writer, is there a race condition?");
	const uint32_t sizeBehind = (queueWrapAround - mWrapAroundCounter) * mPayloadSize + (queueLastWritePosition - mLastReadPosition);

	Shutdown();
	mHandler.OnDisconnected("reader was too slow, writer looped around and overwrote reader");
}

void Consumer::ValidateHeader()
{
	//todo: validate header
}

}