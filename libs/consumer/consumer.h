#pragma once

#include <idl/fastq.h>
#include <core/types.h>
#include <core/throw_if.h>
#include <core/logger.h>

#include <string>
#include <fstream>
#include <utility>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

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
		mFastQBuffer = LoadMmappedFile(mShmFilename, mFilesizeMb);
		mFastQueue = reinterpret_cast<FastQueue*>(mFastQBuffer.mAddr);
		LOG(INFO, LM_CONSUMER, "protocol name: " << mFastQueue->mHeader.mProtocolName)
	}

private:
	using FastQueue = Idl::FastQueue<SizeT, CountT>;

	MmapedFile LoadMmappedFile(const std::string& shmFilename, int filesizeMb)
	{
		LOG(INFO, LM_CONSUMER, "loading mmapped file " << shmFilename << " to memory with size of " << filesizeMb << "MB");

		int fd = open(shmFilename.c_str(), O_RDONLY);
		if (fd == -1)
		{
			THROW_IF(true, "Shm file does not exist");
		}

		const int filesize = filesizeMb * 1000 * 1000;
		void* addr = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, 0);
		if (addr == MAP_FAILED)
		{
			close(fd);
			perror("Error mmapping the file");
			exit(EXIT_FAILURE);
		}

		LOG(INFO, LM_CONSUMER, "mapped " << shmFilename << " to memory successfully");
		return MmapedFile
		{
			.mFd = fd,
			.mAddr = addr
		};
	}

private:
	std::string mShmFilename;
	int mFilesizeMb;
	MmapedFile mFastQBuffer;

	FastQueue* mFastQueue;
};

}