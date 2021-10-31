#include "consumer.h"

#include <core/throw_if.h>
#include <core/logger.h>
#include <idl/fastq_generated.h>
#include <idl/system_asserts.h> // do not remove, ensures important assertions
#include <flatbuffers/flatbuffers.h>

#include <fstream>
#include <utility>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace FastQ {

static const LogModule LM_CONSUMER {"FASTQ_CONSUMER"};

Consumer::Consumer(std::string shmFilename, int filesizeMb)
	: mShmFilename(std::move(shmFilename))
	, mFilesizeMb(filesizeMb)
{

}

void Consumer::Start()
{
	LOG(INFO, LM_CONSUMER, "starting consumer")

	mFd = LoadMmappedFile(mShmFilename, mFilesizeMb);

	flatbuffers::FlatBufferBuilder builder(1024);
	auto container = FastQContainer(1.0f, 2.0f, 3.0f);

//	std::ifstream infile;
//	infile.open("monsterdata_test.mon", std::ios::binary | std::ios::in);
//	infile.seekg(0,std::ios::end);
//	int length = infile.tellg();
//	infile.seekg(0,std::ios::beg);
//	char *data = new char[length];
//	infile.read(data, length);
//	infile.close();

	//auto monster = GetMonster(data);
}

int Consumer::LoadMmappedFile(const std::string& shmFilename, int filesizeMb)
{
	LOG(INFO, LM_CONSUMER, "loading mmapped file " << shmFilename << " to memory with size of " << filesizeMb << "MB");

	int fd = open(shmFilename.c_str(), O_RDONLY);
	if (fd == -1)
	{
		THROW_IF(true, "Shm file does not exist");
	}

	const int filesize = filesizeMb * 1000 * 1000;
	void* map = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}

	LOG(INFO, LM_CONSUMER, "mapped " << shmFilename << " to memory successfully");
	return fd;
}

}