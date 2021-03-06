#pragma once

#include <core/mmap_file.h>
#include <core/fastq_core.h>
#include <core/throw_if.h>
#include <idl/fastq.h>
#include <memory>
#include <cstring>
#include <random>
#include <array>

namespace FastQ {

class Producer : public FastQCore
{
public:
	Producer(std::string shmFilename, int fileSize);

	void Start();
	void Shutdown(); // ensures clean closing of shm
	void Push(uint32_t type, void* data, uint32_t size);

private:
	Idl::FastQueue CreateQueue() const;
	uint32_t WriteData(uint32_t lastWritePosition, void* data, uint32_t size);

private:
	std::string mShmFilename;
	uint32_t mFileSize;
	std::unique_ptr<MmappedFile> mFastQBuffer;
	Idl::FastQueue* mFastQueue;
	uint32_t mLastWritePosition {0};
	uint32_t mWrapAroundCounter {0};

	friend class FastQTestFixture;
};

}