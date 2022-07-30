#pragma once

#include <utility>
#include <functional>

namespace FastQ {

template <class T, class Tag>
class StrongTypedef
{
public:
	StrongTypedef() :
		mValue{}
	{}

	explicit StrongTypedef(const T& value) :
		mValue(value)
	{}

	explicit StrongTypedef(T&& value) :
		mValue(std::move(value))
	{}

	StrongTypedef& operator+=(const StrongTypedef& rhs) { mValue += rhs.mValue; return *this; }
	StrongTypedef operator+(const StrongTypedef& rhs) const { return StrongTypedef{mValue + rhs.mValue}; }

	StrongTypedef& operator-=(const StrongTypedef& rhs) { mValue -= rhs.mValue; return *this; }
	StrongTypedef operator-(const StrongTypedef& rhs) const { return StrongTypedef{mValue - rhs.mValue}; }

	StrongTypedef operator-() const { return StrongTypedef{-mValue}; }

	bool operator<(const StrongTypedef& rhs) const { return mValue < rhs.mValue; }
	bool operator>(const StrongTypedef& rhs) const { return mValue > rhs.mValue; }
	bool operator==(const StrongTypedef& rhs) const { return mValue == rhs.mValue; }
	bool operator!=(const StrongTypedef& rhs) const { return mValue != rhs.mValue; }

	T mValue;
};

template <class Stream, class T, class Tag>
Stream& operator<<(Stream& stream, const StrongTypedef<T, Tag>& x)
{
	stream << x.mValue;
	return stream;
}

}

namespace std
{

template <typename T, typename Tag>
struct hash<FastQ::StrongTypedef<T, Tag>>
{
	std::size_t operator()(const FastQ::StrongTypedef<T, Tag>& x) const
	{
		return std::hash<T>{}(x.mValue);
	}
};

}
