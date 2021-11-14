#pragma once

#include "logger.h"
#include <idl/fastq.h>

namespace FastQ {

template<uint32_t FrameSizeT>
class FastQCore {
public:
	virtual ~FastQCore() = default;

	const Idl::Header<FrameSizeT>& GetHeader()
	{
		return mFastQueue->mHeader;
	}

protected:
	explicit FastQCore(const LogModule& module)
		: mLogModule(module)
	{}

	void Init(Idl::FastQueue<FrameSizeT>* fastQueue)
	{
		mFastQueue = fastQueue;
		mMaxFrameCount = mFastQueue->mHeader.mMaxFrameCount;
		LogFastQHeader();
	}

	void LogFastQHeader() const
	{
		LOG(INFO, mLogModule, "fastq header: ");
		LOG(INFO, mLogModule, "  protocol name:   " << mFastQueue->mHeader.mProtocolName)
		LOG(INFO, mLogModule, "  major version:   " << mFastQueue->mHeader.mVersionMajor)
		LOG(INFO, mLogModule, "  minor version:   " << mFastQueue->mHeader.mVersionMinor)
		LOG(INFO, mLogModule, "  magic number:    " << mFastQueue->mHeader.mMagicNumber)
		LOG(INFO, mLogModule, "  frame size:      " << mFastQueue->mHeader.mFrameSize)
		LOG(INFO, mLogModule, "  max frame count: " << mFastQueue->mHeader.mMaxFrameCount)
		LOG(INFO, mLogModule, "  payload size:    " << mFastQueue->mHeader.mPayloadSize)
	}

	int GetTotalFastQSize() const
	{
		return Idl::FASTQ_SIZE_WITHOUT_PAYLOAD + FrameSizeT * mMaxFrameCount;
	}

	inline uint32_t GetOffset(uint32_t index) const
	{
		return FrameSizeT * (index % mMaxFrameCount);
	}

	inline uint32_t NextFrameIndex(int32_t index) const
	{
		return (index + 1) % mMaxFrameCount;
	}

protected:
	const LogModule& mLogModule;
	Idl::FastQueue<FrameSizeT>* mFastQueue;
	uint32_t mMaxFrameCount {0};
};

}