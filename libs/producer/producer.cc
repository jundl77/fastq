#include "producer.h"

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

static const LogModule LM_PRODUCER {"FASTQ_PRODUCER"};

Producer::Producer(std::string shmFilename, int filesizeMb)
	: mShmFilename(std::move(shmFilename))
	, mFilesizeMb(filesizeMb)
{

}

void Producer::Start()
{
	LOG(INFO, LM_PRODUCER, "starting producer")

	mFd = CreateMmappedFile(mShmFilename, mFilesizeMb);

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

int Producer::CreateMmappedFile(const std::string& shmFilename, int filesizeMb)
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
	return fd;
}

}