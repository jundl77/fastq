#include "producer.h"

#include <core/throw_if.h>
#include <core/logger.h>
#include <idl/fastq.h>

#include <fstream>
#include <utility>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>

namespace FastQ {

static const LogModule LM_PRODUCER {"FASTQ_PRODUCER"};

Producer::Producer(std::string shmFilename, int filesizeMb)
	: mShmFilename(std::move(shmFilename))
	, mFilesizeMb(filesizeMb)
{

}

void Producer::Start()
{
	LOG(INFO, LM_PRODUCER, "starting producer")

	mFastQBuffer = CreateMmappedFile(mShmFilename, mFilesizeMb);
	Idl::FastQueue fastQueue {};
	memcpy(mFastQBuffer.mAddr, &fastQueue, sizeof(fastQueue));
	mFastQueue = reinterpret_cast<Idl::FastQueue*>(mFastQBuffer.mAddr);

	mFastQueue->mTestValue = 10;
	LOG(INFO, LM_PRODUCER, "test value: " << (int) mFastQueue->mTestValue)
}

MmapedFile Producer::CreateMmappedFile(const std::string& shmFilename, int filesizeMb)
{
	LOG(INFO, LM_PRODUCER, "mapping " << shmFilename << " to memory with size of " << filesizeMb << "MB");

	// open file
	int fd = open(shmFilename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
	if (fd == -1)
	{
		THROW_IF(true, "Error opening file for writing.");
	}

	// stretch file size to desired size
	const int filesize = filesizeMb * 1000 * 1000;
	if (lseek(fd, filesize, SEEK_SET) == -1)
	{
		close(fd);
		THROW_IF(true, "Error calling lseek() to 'stretch' the file");
	}

	// write zero-byte at end of file to have the file actually have the new size
	if (write(fd, "", 1) != 1)
	{
		close(fd);
		THROW_IF(true, "Error writing last byte of the file");
	}

	// seek file back to 0
	if (lseek(fd, 0, SEEK_SET) == -1)
	{
		close(fd);
		THROW_IF(true, "Error calling lseek() to reset seek to 0");
	}

	// mmap the file
	void* addr = mmap(nullptr, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		close(fd);
		THROW_IF(true, "Error mmapping the file");
	}

	LOG(INFO, LM_PRODUCER, "mapped " << shmFilename << " to memory successfully");
	return MmapedFile
	{
		.mFd = fd,
		.mAddr = addr
	};
}

}