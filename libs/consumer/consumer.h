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
	Consumer(std::string shmFilename, int filesizeMb)
		: mShmFilename(std::move(shmFilename))
		, mFilesizeMb(filesizeMb)
	{}

	void Start()
	{
		LOG(INFO, LM_CONSUMER, "starting consumer")
		mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
		mFastQBuffer->Create(mFilesizeMb * 1000 * 1000);
		mFastQBuffer->Mmap(mFilesizeMb * 1000 * 1000, MmapProtMode::READ_ONLY);
		mFastQueue = reinterpret_cast<FastQueue*>(mFastQBuffer->GetAddress());
		LOG(INFO, LM_CONSUMER, "protocol name: " << mFastQueue->mHeader.mProtocolName);
	}

private:
	using FastQueue = Idl::FastQueue<SizeT, CountT>;

private:
	std::string mShmFilename;
	int mFilesizeMb;
	std::unique_ptr<MmappedFile> mFastQBuffer;

	FastQueue* mFastQueue;
};

}