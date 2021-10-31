#pragma once

#include <boost/static_string/static_string.hpp>
#include <cstddef>
#include <cstdint>

namespace FastQ::Idl {

static constexpr size_t FASTQ_HEADER_SIZE = 43;
static constexpr uint32_t FASTQ_MAJOR_VERSION = 1;
static constexpr uint32_t FASTQ_MINOR_VERSION = 1;
static const boost::static_string<16> FASTQ_PROTOCOL_NAME {"fastq_protocol"};

#pragma pack(push, 1)

template<uint32_t SizeT, uint32_t CountT>
struct Header
{
	boost::static_string<16> mProtocolName {FASTQ_PROTOCOL_NAME}; // 2 bytes longer: 16 + 2 -> size 18
	uint32_t mVersionMajor {FASTQ_MAJOR_VERSION};
	uint32_t mVersionMinor {FASTQ_MINOR_VERSION};
	uint8_t mMagicNumber {0};
	uint32_t mItemSize {SizeT};
	uint32_t mItemCount {CountT};
	uint64_t mPayloadSize {SizeT * CountT};
};
static_assert(sizeof(Header<1, 1>) == FASTQ_HEADER_SIZE);

template<uint32_t SizeT, uint32_t CountT>
struct FastQueue
{
	Header<SizeT, CountT> mHeader;
	uint8_t mPayload[SizeT * CountT];
};

#pragma pack(pop)

}