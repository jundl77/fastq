#pragma once

#include <core/mmap_file.h>
#include <core/fastq_utils.h>
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

template<uint32_t SizeT, uint32_t CountT>
class Producer
{
public:
	Producer(std::string shmFilename, int filesize)
		: mShmFilename(std::move(shmFilename))
		, mFilesize(filesize)
	{}

	void Start()
	{
		LOG(INFO, LM_PRODUCER, "starting producer")

		Idl::FastQueue queue = CreateQueue();

		mFastQBuffer = std::make_unique<MmappedFile>(mShmFilename);
		mFastQBuffer->Create(mFilesize);
		mFastQBuffer->Mmap(mFilesize, MmapProtMode::READ_WRITE);

		std::memcpy(mFastQBuffer->GetAddress(), &queue, sizeof(queue));
		mFastQueue = reinterpret_cast<FastQueue*>(mFastQBuffer->GetAddress());
		LogFastQHeader<SizeT, CountT>(LM_PRODUCER, mFastQueue);
	}

	void Poll_100ms()
	{
		UpdateHeartbeat();
	}

	void Push(void* data, int size)
	{

	}

private:
	using FastQueue = Idl::FastQueue<SizeT, CountT>;

	FastQueue CreateQueue() const
	{
		THROW_IF(mFilesize < SizeT * CountT, "total filesize is smaller than item_size * item_count");
		std::random_device randomDevice;
		std::mt19937 rng(randomDevice());
		std::uniform_int_distribution<std::mt19937::result_type> dist(1, std::numeric_limits<uint32_t>::max());

		FastQueue fastQueue {};
		fastQueue.mHeader.mMagicNumber = dist(rng);
		return fastQueue;
	}

	void UpdateHeartbeat()
	{
		mFastQueue->mHeader.mHeartbeatCount += 1;
	}

private:
	std::string mShmFilename;
	int mFilesize;
	std::unique_ptr<MmappedFile> mFastQBuffer;

	FastQueue* mFastQueue;
};

}