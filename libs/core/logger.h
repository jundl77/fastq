#pragma once

#include "strong_typedef.h"
#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <chrono>

#define DEBUG 0
#define INFO  1
#define WARN  2
#define ERROR 3

using LogModule = StrongTypedef<std::string, struct LogModuleTag>;

static uint8_t GLOBAL_LOG_LEVEL = INFO;

using Timestamp = std::chrono::system_clock::time_point;
using NanoPosixTime = int64_t;

inline void SetGlobalLogLevel(int logLevel)
{
	GLOBAL_LOG_LEVEL = logLevel;
}

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

#define LOG(level, module, message) \
	do \
	{ \
		if (level == DEBUG && GLOBAL_LOG_LEVEL == DEBUG) \
		{ \
			std::cout << GetLocalTime() << " [Debug ] [" << module << "] " << message << std::endl; \
		} \
		else if (level == INFO) \
		{ \
			std::cout << GetLocalTime() << " [Info ] [" << module << "] " << message << std::endl; \
		} \
		else if (level == WARN) \
		{ \
			std::cout << GetLocalTime() << " [Warn ] [" << module << "] " << message << std::endl; \
		} \
		else if (level == ERROR) \
		{ \
			std::cout << GetLocalTime() << " [Error ] [" << module << "] " << message << std::endl; \
		} \
	} while(false);
