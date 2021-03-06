#pragma once

namespace le
{
// Encapsulates Zero initialised T (primitive type)
template <typename T, T Zero = 0>
struct TZero final
{
	T handle;

	TZero(T = Zero);
	TZero(TZero&&) noexcept;
	TZero& operator=(TZero&&) noexcept;
	TZero(TZero const&);
	TZero& operator=(TZero const&);
	~TZero();

	operator T() const;
};

template <typename T, T Zero>
TZero<T, Zero>::TZero(T h) : handle(h)
{
}

template <typename T, T Zero>
TZero<T, Zero>::TZero(TZero&& rhs) noexcept
{
	*this = std::move(rhs);
}

template <typename T, T Zero>
TZero<T, Zero>& TZero<T, Zero>::operator=(TZero&& rhs) noexcept
{
	handle = rhs.handle;
	rhs.handle = Zero;
	return *this;
}

template <typename T, T Zero>
TZero<T, Zero>::TZero(TZero const& rhs) = default;

template <typename T, T Zero>
TZero<T, Zero>& TZero<T, Zero>::operator=(TZero const& rhs) = default;

template <typename T, T Zero>
TZero<T, Zero>::~TZero() = default;

template <typename T, T Zero>
TZero<T, Zero>::operator T() const
{
	return handle;
}
} // namespace le
