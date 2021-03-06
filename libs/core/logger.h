#pragma once

#include "strong_typedef.h"
#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <chrono>

namespace FastQ {

#define FASTQ_DEBUG 0
#define FASTQ_INFO  1
#define FASTQ_WARN  2
#define FASTQ_ERROR 3

using LogModule = StrongTypedef<std::string, struct LogModuleTag>;

using Timestamp = std::chrono::system_clock::time_point;
using NanoPosixTime = int64_t;

void SetGlobalLogLevel(int logLevel);
uint8_t GetGlobalLogLevel();

inline NanoPosixTime ToNanoPosixTime(const Timestamp& timestamp)
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp.time_since_epoch()).count();
}

inline NanoPosixTime CurrentNanoPosixTime()
{
	return ToNanoPosixTime(std::chrono::system_clock::now());
}

inline decltype(auto) GetLocalTime()
{
	const int64_t kT_ns_in_s = 1000000000;
	const size_t max_size = 35;
	auto time = CurrentNanoPosixTime();
	std::string result(max_size, ' ');

	time_t secs = (time_t)(time / kT_ns_in_s);
	struct tm tm_;
	size_t offset;
	localtime_r(&secs, &tm_);
	offset = strftime(result.data(), max_size, "%Y-%m-%d %H:%M:%S", &tm_);
	offset += sprintf(result.data() + offset, ".%09llu", static_cast<unsigned long long>((time % kT_ns_in_s)));
	result.resize(offset);
	return result;
}

#define LOG(level, module, ...) \
	do \
	{ \
		if (level == FASTQ_DEBUG && GetGlobalLogLevel() == FASTQ_DEBUG) \
		{ \
			std::cout << GetLocalTime() << " [Debug ] [" << module << "] "; \
			std::printf(__VA_ARGS__); \
			std::cout << std::endl; \
		} \
		else if (level == FASTQ_INFO && GetGlobalLogLevel() <= FASTQ_INFO) \
		{ \
			std::cout << GetLocalTime() << " [Info ] [" << module << "] "; \
			std::printf(__VA_ARGS__); \
			std::cout << std::endl; \
		} \
		else if (level == FASTQ_WARN && GetGlobalLogLevel() <= FASTQ_WARN) \
		{ \
			std::cout << GetLocalTime() << " [Warn ] [" << module << "] "; \
			std::printf(__VA_ARGS__); \
			std::cout << std::endl; \
		} \
		else if (level == FASTQ_ERROR && GetGlobalLogLevel() <= FASTQ_ERROR) \
		{ \
			std::cout << GetLocalTime() << " [Error ] [" << module << "] "; \
			std::printf(__VA_ARGS__); \
			std::cout << std::endl; \
		} \
	} while(false);

#ifdef NDEBUG
	#define DEBUG_LOG(level, module, ...) {}
#else
	#define DEBUG_LOG(level, module, ...) { LOG(level, module, __VA_ARGS__) }
#endif

}