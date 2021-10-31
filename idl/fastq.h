#pragma once

#include <cstddef>
#include <cstdint>

#define FASTQ_PROTOCOL_NAME "fastq_protocol"

namespace FastQ::Idl {

static constexpr size_t FASTQ_HEADER_SIZE = 48;
static constexpr size_t FASTQ_SIZE_WITHOUT_PAYLOAD = FASTQ_HEADER_SIZE;
static constexpr uint32_t FASTQ_MAJOR_VERSION = 1;
static constexpr uint32_t FASTQ_MINOR_VERSION = 1;

#pragma pack(push, 1)

template<uint32_t SizeT, uint32_t CountT>
struct Header
{
	char mProtocolName[16] = FASTQ_PROTOCOL_NAME;
	uint32_t mVersionMajor {FASTQ_MAJOR_VERSION};
	uint32_t mVersionMinor {FASTQ_MINOR_VERSION};
	uint32_t mMagicNumber {0};
	uint32_t mHeartbeatCount {0};
	uint32_t mItemSize {SizeT};
	uint32_t mMaxItemCount {CountT};
	uint64_t mPayloadSize {SizeT * CountT};
};
static_assert(sizeof(Header<1, 1>) == FASTQ_HEADER_SIZE);

template<uint32_t SizeT, uint32_t CountT>
struct FastQueue
{
	Header<SizeT, CountT> mHeader;
	uint8_t mPayload[sizeof(SizeT) * CountT];
};
static_assert(FASTQ_HEADER_SIZE == FASTQ_SIZE_WITHOUT_PAYLOAD);

#pragma pack(pop)

}