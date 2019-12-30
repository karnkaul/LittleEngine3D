#pragma once

namespace le
{
template <typename T, s32 Z = 0>
struct TZero
{
	T handle = Z;

	TZero();
	TZero(T);
	TZero(TZero&&);
	TZero& operator=(TZero&&);
	TZero(const TZero&);
	TZero& operator=(const TZero&);
	~TZero();

	operator T const()
	{
		return handle;
	}
};

template <typename T, s32 Z>
TZero<T, Z>::TZero() = default;

template <typename T, s32 Z>
TZero<T, Z>::TZero(T h) : handle(h)
{
}

template <typename T, s32 Z>
TZero<T, Z>::TZero(TZero&& rhs)
{
	*this = std::move(rhs);
}

template <typename T, s32 Z>
TZero<T, Z>& TZero<T, Z>::operator=(TZero&& rhs)
{
	handle = rhs.handle;
	rhs.handle = Z;
	return *this;
}

template <typename T, s32 Z>
TZero<T, Z>::TZero(const TZero& rhs) = default;

template <typename T, s32 Z>
TZero<T, Z>& TZero<T, Z>::operator=(const TZero& rhs) = default;

template <typename T, s32 Z>
TZero<T, Z>::~TZero() = default;
} // namespace le
