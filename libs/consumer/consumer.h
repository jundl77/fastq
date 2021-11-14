#pragma once

#include <core/mmap_file.h>
#include <core/fastq_core.h>
#include <core/throw_if.h>
#include <core/logger.h>
#include <idl/fastq.h>
#include <memory>
#include <cstring>

namespace {
	const LogModule LM_CONSUMER {"FASTQ_CONSUMER"};
}

namespace FastQ {

static constexpr double MAX_PERCENT_BEHIND = 0.75;

template<uint32_t FrameSizeT>
class Consumer : public FastQCore<FrameSizeT>
{
public:
	Consumer(std::string shmFilename)
		: FastQCore(LM_CONSUMER)
		, mShmFilename(std::move(shmFilename))
	{}

	void Start()
	{
		LOG(INFO, LM_CONSUMER, "starting consumer")
		mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
		mFastQBuffer->Mmap(FastQCore::GetTotalFastQSize(), MmapProtMode::READ_ONLY);
		mFastQueue = reinterpret_cast<FastQueue*>(mFastQBuffer->GetAddress());
		FastQCore::Init(mFastQueue);
		mMaxFrameCount = mFastQueue->mHeader.mMaxFrameCount;
		mWrapAroundCount = mFastQueue->mWrapAroundCount;
		mLastReadIndex = mFastQueue->mLastWriteIndex;
	}

	bool Pop(void* data)
	{
		const int wrapAroundOffset = std::abs(mFastQueue->mWrapAroundCount - mWrapAroundCount) * mMaxFrameCount;
		const int indexBehindCount = std::abs(mFastQueue->mLastWriteIndex + wrapAroundOffset - mLastReadIndex);
		const double percentBehind = (indexBehindCount * 1.0) / mMaxFrameCount;
		if (indexBehindCount >= 100 && percentBehind > MAX_PERCENT_BEHIND)
		{
			THROW_IF(true, "consumer fell behind by " + std::to_string(percentBehind * 100) + "% capacity  which is more than max (" + std::to_string(MAX_PERCENT_BEHIND * 100) +"%)");
		}
		if (mLastReadIndex == mFastQueue->mLastWriteIndex)
		{
			return false;
		}

		uint32_t readOffset = FastQCore::GetOffset(mLastReadIndex);
		std::memcpy(data, mFastQueue->mPayload + readOffset, mMaxFrameCount);
		mLastReadIndex = FastQCore::NextFrameIndex(mLastReadIndex);
		if (mLastReadIndex == 0)
		{
			mWrapAroundCount++;
		}
		return true;
	}

	const Idl::Header<FrameSizeT>& GetHeader()
	{
		return mFastQueue->mHeader;
	}

private:
	using FastQCore = FastQCore<FrameSizeT>;
	using FastQueue = Idl::FastQueue<FrameSizeT>;

	std::string mShmFilename;
	std::unique_ptr<MmappedFile> mFastQBuffer;

	FastQueue* mFastQueue;
	uint32_t mMaxFrameCount {0};
	int32_t mWrapAroundCount {0};
	int32_t mLastReadIndex {0};
};

}