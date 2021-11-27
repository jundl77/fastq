#pragma once

#include <stdexcept>
#include <cstdio>

#define THROW_IF(cond, ...) \
	do \
	{ \
		if (cond) \
		{ \
			char buffer[512]; \
			std::sprintf(buffer, __VA_ARGS__); \
			throw std::runtime_error(buffer); \
		} \
	} while(false);

#ifdef NDEBUG
	#define DEBUG_THROW_IF(cond, ...) {}
#else
	#define DEBUG_THROW_IF(cond, ...) { THROW_IF(cond, __VA_ARGS__) }
#endif