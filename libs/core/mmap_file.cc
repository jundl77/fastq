#include "mmap_file.h"

#include <core/logger.h>
#include <core/throw_if.h>

#include <string>
#include <cstring>
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
	Close();
}

void MmappedFile::Create(int initialSize)
{
	THROW_IF(mIsMapped, "cannot create a new shm file while there is already a file mapped")
	LOG(FASTQ_INFO, LM_MMAP, "creating %s with initial size of %d", mShmFilename.c_str(), initialSize);

	// open file
	mProtMode = MmapProtMode::READ_WRITE;
	mFd = ShmOpenFile(mProtMode);

	// grow file to initial size
	if (-1 == ftruncate(mFd, initialSize))
	{
		LOG(FASTQ_ERROR, LM_MMAP, "ftruncate failed, closing file, errno=%s (%d)", std::strerror(errno), errno);
		CloseFile();
		THROW_IF(true, "unable to grow file to specified size: %d", initialSize)
	}
}

void MmappedFile::Mmap(int size, MmapProtMode prot)
{
	THROW_IF(mIsMapped, "cannot map shm file, file is already mapped")
	LOG(FASTQ_INFO, LM_MMAP, "mapping %s to memory with size %d", mShmFilename.c_str(), size);

	// make sure file exists
	if (0 == mFd)
	{
		mFd = ShmOpenFile(prot);
	}

	// mmap the file
	void* addr = mmap(NULL, size, MmapProtModeToProt(prot), MAP_SHARED, mFd, 0);
	if (MAP_FAILED == addr)
	{
		LOG(FASTQ_ERROR, LM_MMAP, "mmap failed, closing file, errno=%s (%d)", std::strerror(errno), errno);
		CloseFile();
		THROW_IF(true, "error mapping file to memory")
	}

	mAddr = addr;
	mMappedSize = size;
	mIsMapped = true;
	mProtMode = prot;
	LOG(FASTQ_INFO, LM_MMAP, "mapped %s to memory successfully", mShmFilename.c_str());
}

void MmappedFile::Munmap()
{
	THROW_IF(!mIsMapped, "cannot unmap shm file that is not mapped")
	if (-1 == munmap(mAddr, mMappedSize))
	{
		LOG(FASTQ_ERROR, LM_MMAP, "munmap failed, closing file, errno=%s (%d)", std::strerror(errno), errno);
		CloseFile();
		THROW_IF(true, "error unmmapping file");
	}
	mAddr = NULL;
	mMappedSize = 0;
	mIsMapped = false;
	LOG(FASTQ_INFO, LM_MMAP, "unmapped %s from memory successfully", mShmFilename.c_str());
}

void MmappedFile::Close()
{
	if (mIsMapped)
	{
		Munmap();
	}
	if (mIsOpen)
	{
		CloseFile();
	}
}

void* MmappedFile::GetAddress() const
{
	THROW_IF(!mIsMapped, "cannot get address if file is not mapped")
	return mAddr;
}

int MmappedFile::ShmOpenFile(MmapProtMode mode)
{
	int fd = shm_open(mShmFilename.c_str(), MmapProtModeToOMode(mode), (mode_t)0600);
	THROW_IF(-1 == fd, "shm_open failed, errno=%s (%d)", std::strerror(errno), errno);
	mIsOpen = true;
	return fd;
}

void MmappedFile::CloseFile()
{
	if (mProtMode == MmapProtMode::READ_WRITE)
	{
		int res = shm_unlink(mShmFilename.c_str());
		THROW_IF(0 != res, "shm_unlink failed, errno=%s (%d)", std::strerror(errno), errno);
		LOG(FASTQ_INFO, LM_MMAP, "shm_unlinked %s from memory successfully", mShmFilename.c_str());
	}
	close(mFd);
	mIsOpen = false;
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
		return O_RDWR | O_CREAT;
	case MmapProtMode::READ_ONLY:
		return O_RDONLY;
	default:
		THROW_IF(true, "unknown MmapProtMode type")
	}
}

}