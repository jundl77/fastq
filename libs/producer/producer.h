#pragma once

#include <core/mmap_file.h>
#include <core/fastq_core.h>
#include <core/logger.h>
#include <core/throw_if.h>
#include <idl/fastq.h>
#include <memory>
#include <cstring>
#include <random>

namespace {
	const LogModule LM_PRODUCER {"FASTQ_PRODUCER"};
}

namespace FastQ {

template<uint32_t FrameSizeT>
class Producer : public FastQCore<FrameSizeT>
{
public:
	Producer(std::string shmFilename, int payloadSize)
		: FastQCore(LM_PRODUCER)
		, mShmFilename(std::move(shmFilename))
		, mPayloadSize(payloadSize)
		, mFilesize(Idl::FASTQ_SIZE_WITHOUT_PAYLOAD + mPayloadSize)
	{
	}

	void Start()
	{
		LOG(INFO, LM_PRODUCER, "starting producer")

		Idl::FastQueue queue = CreateQueue();

		mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
		mFastQBuffer->Create(mFilesize);
		mFastQBuffer->Mmap(mFilesize, MmapProtMode::READ_WRITE);

		std::memcpy(mFastQBuffer->GetAddress(), &queue, sizeof(queue));
		mFastQueue = reinterpret_cast<FastQueue*>(mFastQBuffer->GetAddress());
		FastQCore::Init(mFastQueue);
	}

	void Poll_100ms()
	{
		UpdateHeartbeat();
	}

	void Push(void* data, int size)
	{
		// dont increment until after we wrote to the queue
		const uint32_t nextIndex = FastQCore::NextFrameIndex(mFastQueue->mLastWriteIndex);
		const uint32_t writeAddr = FastQCore::GetOffset(nextIndex);
		std::memcpy(mFastQueue->mPayload + writeAddr, data, size);

		// increment now
		mFastQueue->mLastWriteIndex = nextIndex;
		if (mFastQueue->mLastWriteIndex == 0)
			mFastQueue->mWrapAroundCount++;
	}

private:
	Idl::FastQueue<FrameSizeT> CreateQueue() const
	{
		const uint32_t maxFrameCount = (uint32_t) (mPayloadSize) / FrameSizeT;
		std::random_device randomDevice;
		std::mt19937 rng(randomDevice());
		std::uniform_int_distribution<std::mt19937::result_type> dist(1, std::numeric_limits<uint32_t>::max());
		LOG(INFO, LM_PRODUCER, "creating fastq with max frame count: " << maxFrameCount)
		return Idl::FastQueue<FrameSizeT>{dist(rng), maxFrameCount};
	}

	void UpdateHeartbeat()
	{
		mFastQueue->mHeartbeatCount += 1;
	}

private:
	using FastQCore = FastQCore<FrameSizeT>;
	using FastQueue = Idl::FastQueue<FrameSizeT>;

	std::string mShmFilename;
	int mPayloadSize;
	int mFilesize;
	std::unique_ptr<MmappedFile> mFastQBuffer;
	FastQueue* mFastQueue;
};

}