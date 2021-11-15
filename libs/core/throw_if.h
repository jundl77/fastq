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
