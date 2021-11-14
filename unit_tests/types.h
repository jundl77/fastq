#pragma once

#include <cstddef>
#include <cstdint>

namespace FastQ::Testing {

#pragma pack(push, 1)

struct SimpleFoo
{
	uint32_t mFoo1 {0};
	uint32_t mFoo2 {0};
	uint32_t mFoo3 {0};
};

#pragma pack(pop)

}