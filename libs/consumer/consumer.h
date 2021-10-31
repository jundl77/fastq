#pragma once

#include <core/mmap_file.h>
#include <core/logger.h>
#include <idl/fastq.h>
#include <memory>
#include <cstring>

namespace {
	const LogModule LM_CONSUMER {"FASTQ_CONSUMER"};
}

namespace FastQ {

template<uint32_t SizeT, uint32_t CountT>
class Consumer
{
public:
	Consumer(std::string shmFilename)
		: mShmFilename(std::move(shmFilename))
	{}

	void Start()
	{
		LOG(INFO, LM_CONSUMER, "starting consumer")
		mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
		mFastQBuffer->Mmap(TotalBufferSize(), MmapProtMode::READ_ONLY);
		mFastQueue = reinterpret_cast<FastQueue*>(mFastQBuffer->GetAddress());
		LOG(INFO, LM_CONSUMER, "protocol name: " << mFastQueue->mHeader.mProtocolName);
		LOG(INFO, LM_CONSUMER, "magic number: " << mFastQueue->mHeader.mMagicNumber);
	}

private:
	using FastQueue = Idl::FastQueue<SizeT, CountT>;

	int TotalBufferSize()
	{
		return Idl::FASTQ_SIZE_WITHOUT_PAYLOAD + SizeT * CountT;
	}

	std::string mShmFilename;
	std::unique_ptr<MmappedFile> mFastQBuffer;

	FastQueue* mFastQueue;
};

}