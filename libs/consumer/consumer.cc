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

	FastQCore::Init(mFastQueue);
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo;
	mWrapAroundCount = GetWrapAroundCount(lastWriteInfo);
	mLastReadPosition = GetLastWritePosition(lastWriteInfo);
	LOG(INFO, LM_CONSUMER, "position in queue:")
	LOG(INFO, LM_CONSUMER, "  wrap around count:  %d", mWrapAroundCount)
	LOG(INFO, LM_CONSUMER, "  last read position: %d", mLastReadPosition)
}

bool Consumer::Poll()
{
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo;
	const uint32_t queueWrapAround = GetWrapAroundCount(lastWriteInfo);
	const uint32_t queueLastWritePosition = GetLastWritePosition(lastWriteInfo);
	if (mLastReadPosition == queueLastWritePosition && mWrapAroundCount == queueWrapAround)
	{
		return false;
	}
	DEBUG_THROW_IF(mWrapAroundCount > queueWrapAround, "wrap count of reader is ahead of that of writer, is there a race condition?");
	DEBUG_THROW_IF(mLastReadPosition > queueLastWritePosition && mWrapAroundCount >= queueWrapAround,
				"reader is ahead of writer, is there a race condition?");

	// no need to check for in-sync at start, because if we are not, we will:
	// - either read a garbage header with an invalid size, in which case we throw
	// - or read a garbage header with a valid size, in which case we copy at most the entire queue,
	//   then check if we are in-sync at the end, and then throw

	Idl::FramingHeader framingHeader;
	mLastReadPosition = ReadData(mLastReadPosition, &framingHeader, Idl::FASTQ_FRAMING_HEADER_SIZE);

	// check if the header has a valid size here, to make sure we can copy, even if we were overtaken
	THROW_IF(framingHeader.mSize > mPayloadSize, "consumer was too slow, producer overwrote data");
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
	const auto endReadPosition = FastQCore::NextFramePosition(lastReadPosition, size);

	// if we have wrapped around, set writeAddr to the start of the payload again, otherwise keep going
	uint8_t* readAddr = payload + (endReadPosition - size);
	if (endReadPosition == 0)
	{
		mWrapAroundCount++;
		readAddr = payload;
		DEBUG_LOG(INFO, LM_CONSUMER, "consumer wrapper around, wrap-around counter: %d", mWrapAroundCount)
	}

	std::memcpy(data, readAddr, size);
	return endReadPosition;
}

void Consumer::AssertInSync()
{
	const uint64_t lastWriteInfo = mFastQueue->mLastWriteInfo;
	const uint32_t queueWrapAround = GetWrapAroundCount(lastWriteInfo);
	const uint32_t queueLastWritePosition = GetLastWritePosition(lastWriteInfo);

	DEBUG_THROW_IF(mWrapAroundCount > queueWrapAround, "wrap count of reader is ahead of that of writer, is there a race condition?");
	DEBUG_THROW_IF(mLastReadPosition > queueLastWritePosition && mWrapAroundCount >= queueWrapAround,
				   "reader is ahead of writer, is there a race condition?");
	const uint32_t sizeBehind = (queueWrapAround - mWrapAroundCount) * mPayloadSize + (queueLastWritePosition - mLastReadPosition);
	THROW_IF(sizeBehind >= mPayloadSize, "reader was too slow, writer looped around and overwrote reader");
}

void Consumer::ValidateHeader()
{
	//todo: validate header
}

}