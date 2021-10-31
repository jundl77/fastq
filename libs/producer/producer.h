#pragma once

#include <core/mmap_file.h>
#include <core/logger.h>
#include <idl/fastq.h>
#include <memory>
#include <cstring>

namespace {
	const LogModule LM_PRODUCER {"FASTQ_PRODUCER"};
}

namespace FastQ {

template<uint32_t SizeT, uint32_t CountT>
class Producer
{
public:
	Producer(std::string shmFilename, int filesizeMb)
		: mShmFilename(std::move(shmFilename))
		, mFilesizeMb(filesizeMb)
		{}

	void Start()
	{
		LOG(INFO, LM_PRODUCER, "starting producer")

		Idl::FastQueue fastQueue = CreateFastQueue();

		mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
		mFastQBuffer->Create(mFilesizeMb * 1000 * 1000);
		mFastQBuffer->Mmap(mFilesizeMb * 1000 * 1000, MmapProtMode::READ_WRITE);

		std::memcpy(mFastQBuffer->GetAddress(), &fastQueue, sizeof(fastQueue));
		mFastQueue = reinterpret_cast<FastQueue*>(mFastQBuffer->GetAddress());
		LOG(INFO, LM_PRODUCER, "protocol name: " << mFastQueue->mHeader.mProtocolName)
	}

private:
	using FastQueue = Idl::FastQueue<SizeT, CountT>;

	FastQueue CreateFastQueue() const
	{
		FastQueue fastQueue {};
		fastQueue.mHeader.mMagicNumber = 0;
		return fastQueue;
	}

private:
	std::string mShmFilename;
	int mFilesizeMb;
	std::unique_ptr<MmappedFile> mFastQBuffer;

	FastQueue* mFastQueue;
};

}