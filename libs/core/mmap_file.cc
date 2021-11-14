#include "mmap_file.h"

#include <core/logger.h>
#include <core/throw_if.h>

#include <string>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace FastQ {

static const LogModule LM_MMAP {"FASTQ_MMAP"};

MmappedFile::MmappedFile(std::string shmFilename)
	: mShmFilename(std::move(shmFilename))
{
}

MmappedFile::~MmappedFile()
{
	if (mIsMapped)
	{
		Munmap();
	}
}

void MmappedFile::Create(int initialSize)
{
	THROW_IF(mIsMapped, "cannot create a new shm file while there is already a file mapped")
	LOG(INFO, LM_MMAP, "creating " << mShmFilename << " with initial size of " << initialSize);

	// open file
	mFd = ShmOpenFile(MmapProtMode::READ_WRITE);

	// grow file to initial size
	if (lseek(mFd, initialSize - 1, SEEK_SET) == -1)
	{
		close(mFd);
		THROW_IF(true, "error calling lseek() to grow file to initial size");
	}

	// write zero-byte at end of file to make file take on new size
	if (write(mFd, "", 1) != 1)
	{
		close(mFd);
		THROW_IF(true, "error writing last byte of the file");
	}

	// reset seek to 0
	if (lseek(mFd, 0, SEEK_SET) == -1)
	{
		close(mFd);
		THROW_IF(true, "error calling lseek() to reset seek to 0");
	}
}

void MmappedFile::Mmap(int size, MmapProtMode prot)
{
	THROW_IF(mIsMapped, "cannot map shm file, file is already mapped")
	LOG(INFO, LM_MMAP, "mapping " << mShmFilename << " to memory with size " << size);

	// make sure file exists
	if (mFd == 0)
	{
		mFd = ShmOpenFile(prot);
	}

	// mmap the file
	void* addr = mmap(NULL, size, MmapProtModeToProt(prot), MAP_SHARED | MAP_FILE, mFd, 0);
	if (addr == MAP_FAILED)
	{
		CloseFile();
		THROW_IF(true, "error mmapping file, errno=" + std::string(std::strerror(errno)) + " (" + std::to_string(errno) + ")");
	}

	mAddr = addr;
	mMappedSize = size;
	mIsMapped = true;
	LOG(INFO, LM_MMAP, "mapped " << mShmFilename << " to memory successfully");
}

void MmappedFile::Munmap()
{
	THROW_IF(!mIsMapped, "cannot unmap shm file that is not mapped")
	if (munmap(mAddr, mMappedSize) == -1)
	{
		CloseFile();
		THROW_IF(true, "error unmmapping file");
	}
	CloseFile();
	mAddr = NULL;
	mMappedSize = 0;
	mIsMapped = false;
}

void MmappedFile::CloseFile()
{
	close(mFd);
	//shm_unlink(mShmFilename.c_str()); todo:fix
}

void* MmappedFile::GetAddress() const
{
	THROW_IF(!mIsMapped, "cannot get address if file is not mapped")
	return mAddr;
}

int MmappedFile::ShmOpenFile(MmapProtMode mode)
{
	//todo: fix later to shm_open
	int fd = open(mShmFilename.c_str(), MmapProtModeToOMode(mode), (mode_t)0600);
	if (-1 == fd)
	{
		THROW_IF(true, "shm file does not exist");
	}
	return fd;
}

int MmappedFile::MmapProtModeToProt(MmapProtMode mode)
{
	switch (mode)
	{
	case MmapProtMode::READ_WRITE:
		return PROT_READ | PROT_WRITE;
	case MmapProtMode::READ_ONLY:
		return PROT_READ;
	default:
		THROW_IF(true, "unknown MmapProtMode type")
	}
}

int MmappedFile::MmapProtModeToOMode(MmapProtMode mode)
{
	switch (mode)
	{
	case MmapProtMode::READ_WRITE:
		return O_RDWR | O_CREAT | O_TRUNC;
	case MmapProtMode::READ_ONLY:
		return O_RDONLY;
	default:
		THROW_IF(true, "unknown MmapProtMode type")
	}
}

}