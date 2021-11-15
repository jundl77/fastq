#include "producer.h"
#include <core/logger.h>
#include <array>

namespace FastQ {

const LogModule LM_PRODUCER {"FASTQ_PRODUCER"};

Producer::Producer(std::string shmFilename, int fileSize)
	: FastQCore(LM_PRODUCER)
	, mShmFilename(std::move(shmFilename))
	, mFilesize(fileSize)
	, mPayloadSize(mFilesize - Idl::FASTQ_SIZE_WITHOUT_PAYLOAD)
{
}

void Producer::Start()
{
	LOG(INFO, LM_PRODUCER, "starting producer")

	Idl::FastQueue queue = CreateQueue();

	mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
	mFastQBuffer->Create(mFilesize);
	mFastQBuffer->Mmap(mFilesize, MmapProtMode::READ_WRITE);

	std::memset(mFastQBuffer->GetAddress(), 0, mFilesize);
	std::memcpy(mFastQBuffer->GetAddress(), &queue, sizeof(queue));
	mFastQueue = reinterpret_cast<Idl::FastQueue*>(mFastQBuffer->GetAddress());
	FastQCore::Init(mFastQueue);
}

void Producer::Poll_100ms()
{
	UpdateHeartbeat();
}

void Producer::Push(void* data, int size)
{
	// dont increment until after we wrote to the queue
	const uint64_t endAddr = FastQCore::NextFramePosition(mFastQueue->mLastWritePosition, size);
	uint8_t* writeAddr = mFastQueue->mPayload + (endAddr - size);
	if (endAddr == 0)
	{
		mFastQueue->mWrapAroundCount++;
		writeAddr = mFastQueue->mPayload;
	}
	std::memcpy(writeAddr, data, size);

	// increment now
	mFastQueue->mLastWritePosition = endAddr;
}

Idl::FastQueue Producer::CreateQueue() const
{
	std::random_device randomDevice;
	std::mt19937 rng(randomDevice());
	std::uniform_int_distribution<std::mt19937::result_type> dist(1, std::numeric_limits<uint32_t>::max());
	return Idl::FastQueue{dist(rng), mPayloadSize};
}

void Producer::UpdateHeartbeat()
{
	mFastQueue->mHeartbeatCount += 1;
}

}
