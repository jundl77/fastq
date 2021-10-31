#pragma once

#include <idl/fastq.h>
#include <core/types.h>
#include <string>

namespace FastQ {

class Producer
{
public:
	Producer(std::string shmFilename, int filesizeMb);
	void Start();

private:
	MmapedFile CreateMmappedFile(const std::string& shmFilename, int filesizeMb);

	std::string mShmFilename;
	int mFilesizeMb;
	MmapedFile mFastQBuffer;

	Idl::FastQueue* mFastQueue;
};

}