#include "producer.h"
#include <core/logger.h>
#include <array>
#include <cstring>

namespace FastQ {

const LogModule LM_PRODUCER {"FASTQ_PRODUCER"};

Producer::Producer(std::string shmFilename, int fileSize)
	: FastQCore(LM_PRODUCER)
	, mShmFilename(std::move(shmFilename))
	, mFileSize(fileSize)
{
}

void Producer::Start()
{
	LOG(INFO, LM_PRODUCER, "starting producer")

	Idl::FastQueue queue = CreateQueue();

	mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
	mFastQBuffer->Create(mFileSize);
	mFastQBuffer->Mmap(mFileSize, MmapProtMode::READ_WRITE);

	std::memset(mFastQBuffer->GetAddress(), 0, mFileSize);
	std::memcpy(mFastQBuffer->GetAddress(), &queue, sizeof(queue));
	mFastQueue = reinterpret_cast<Idl::FastQueue*>(mFastQBuffer->GetAddress());
	FastQCore::Init(mFastQueue);
}

void Producer::Shutdown()
{
	mFastQBuffer->Close();
}

void Producer::Push(uint32_t type, void* data, uint32_t size)
{
	// dont increment until after we wrote to the queue both times
	const Idl::FramingHeader frame {type, size};
	const auto writePositionAfterFrame = WriteData(mLastWritePosition, (void*)&frame, Idl::FASTQ_FRAMING_HEADER_SIZE);
	const auto lastWritePosition = WriteData(writePositionAfterFrame, data, size);

	// update now
	mLastWritePosition = lastWritePosition;
	mFastQueue->mLastWriteInfo.store(CreateLastWriteInfo(mLastWritePosition, mWrapAroundCounter));
}

uint32_t Producer::WriteData(uint32_t lastWritePosition, void* data, uint32_t size)
{
	uint8_t* payload = GetPayloadPointer();
	uint32_t endWritePosition = FastQCore::NextFramePosition(lastWritePosition, size);

	// if we have wrapped around, set writeAddr to the start of the payload again, otherwise keep going
	uint8_t* writeAddr = payload + (endWritePosition - size);
	if (endWritePosition == 0)
	{
		mWrapAroundCounter++;
		endWritePosition = size;
		writeAddr = payload;
		DEBUG_LOG(INFO, LM_PRODUCER, "producer wrapped around, wrap-around counter: %d", mWrapAroundCounter)
	}

	std::memcpy(writeAddr, data, size);
	DEBUG_LOG(INFO, LM_PRODUCER, "[producer state] last write position %d, wrap-around counter: %d",
			  endWritePosition, mWrapAroundCounter)
	return endWritePosition;
}

Idl::FastQueue Producer::CreateQueue() const
{
	std::random_device randomDevice;
	std::mt19937 rng(randomDevice());
	std::uniform_int_distribution<std::mt19937::result_type> dist(1, std::numeric_limits<uint32_t>::max());
	return Idl::FastQueue{dist(rng), mFileSize};
}

}
