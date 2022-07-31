#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <atomic>

#define FASTQ_PROTOCOL_NAME "fastq_protocol"

namespace FastQ::Idl {

static constexpr uint8_t CACHE_LINE_SIZE = 64;
static constexpr uint32_t FASTQ_SIZE_WITHOUT_PAYLOAD = 128;
static constexpr uint32_t FASTQ_MAJOR_VERSION = 1;
static constexpr uint32_t FASTQ_MINOR_VERSION = 1;

template<typename T>
constexpr bool IsAligned(const T* t)
{
	constexpr auto mask = alignof(T) - 1;
	return (reinterpret_cast<std::uintptr_t>(t) & mask) == 0;
}

constexpr bool IsAligned(const size_t size)
{
	return size % CACHE_LINE_SIZE == 0;
}

#pragma pack(push, 1)

struct alignas(CACHE_LINE_SIZE) FramingHeader
{
	FramingHeader() = default;
	FramingHeader(uint32_t type, uint32_t size)
	: mType(type)
	, mSize(size)
	{}

	uint32_t mType {0};
	uint32_t mSize {0};
};
static_assert(IsAligned(sizeof(FramingHeader)));

struct alignas(CACHE_LINE_SIZE) Header
{
	Header(uint64_t magicNumber, uint32_t fileSize)
		: mMagicNumber(magicNumber)
		, mFileSize(fileSize)
		, mPayloadSize(mFileSize - FASTQ_SIZE_WITHOUT_PAYLOAD)
	{}

	const char mProtocolName[16] = FASTQ_PROTOCOL_NAME;
	const uint32_t mVersionMajor {FASTQ_MAJOR_VERSION};
	const uint32_t mVersionMinor {FASTQ_MINOR_VERSION};
	const uint64_t mMagicNumber {0};
	const uint64_t mChecksum {0}; // todo: calculate and check
	const uint32_t mFileSize {0};
	const uint32_t mPayloadSize {0};
};
static_assert(IsAligned(sizeof(Header)));

struct alignas(CACHE_LINE_SIZE) FastQueue
{
	FastQueue(uint64_t magicNumber, uint32_t fileSize)
		: mHeader(magicNumber, fileSize)
	{
		if (!IsAligned(this))
		{
			std::runtime_error("expected aligned address, got " + std::to_string(reinterpret_cast<std::ptrdiff_t>(this)));
		}
	}

	~FastQueue()
	{
		mLastWriteInfo.store(0, std::memory_order_release);
	}

	const Header mHeader;

	// bits 0-31 contain last write location, bits 32-28 contain wrap around count
	std::atomic<uint64_t> mLastWriteInfo {0};

	// size 0 array, data of N bytes follows
	uint8_t mData[];
};
static_assert(IsAligned(sizeof(FastQueue)));

#pragma pack(pop)

}