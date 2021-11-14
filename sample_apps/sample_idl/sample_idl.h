#pragma once

#include <cstddef>
#include <cstdint>

namespace FastQ::SampleIdl {

#pragma pack(push, 1)

struct SampleData
{
	uint32_t mId {0};
	char mData[128];
};

#pragma pack(pop)

}