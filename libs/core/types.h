#pragma once

namespace FastQ
{

struct MmapedFile
{
	int mFd;
	void* mAddr;
};

}