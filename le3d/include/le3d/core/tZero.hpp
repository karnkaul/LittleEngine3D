#pragma once

namespace le
{
template <typename T, s32 Z = 0>
struct TZero final
{
	T handle = Z;

	TZero();
	TZero(T);
	TZero(TZero&&);
	TZero& operator=(TZero&&);
	TZero(TZero const&);
	TZero& operator=(TZero const&);
	~TZero();

	operator T() const
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
TZero<T, Z>::TZero(TZero const& rhs) = default;

template <typename T, s32 Z>
TZero<T, Z>& TZero<T, Z>::operator=(TZero const& rhs) = default;

template <typename T, s32 Z>
TZero<T, Z>::~TZero() = default;
} // namespace le
