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
	void Poll_100ms();

	void Push(void* data, int size);

private:
	Idl::FastQueue CreateQueue() const;
	void UpdateHeartbeat();

private:
	std::string mShmFilename;
	int mFilesize;
	uint64_t mPayloadSize;
	std::unique_ptr<MmappedFile> mFastQBuffer;
	Idl::FastQueue* mFastQueue;
};

}