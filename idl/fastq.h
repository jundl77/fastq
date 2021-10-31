#pragma once

#include <cstddef>
#include <cstdint>

namespace FastQ::Idl {

static constexpr size_t FASTQ_HEADER_SIZE = 2;
static constexpr size_t FASTQ_QUEUE_SIZE = 33;

#pragma pack(push, 1)

struct Header
{
	uint8_t m;
	uint8_t b;
};
static_assert(sizeof(Header) == FASTQ_HEADER_SIZE);

struct FastQueue
{
	Header mHeader;
	uint8_t mPayload[30];
	uint8_t mTestValue;
};
static_assert(sizeof(FastQueue) == FASTQ_QUEUE_SIZE);

#pragma pack(pop)

}