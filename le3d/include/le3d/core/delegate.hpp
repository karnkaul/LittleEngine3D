/*
 * == LittleEngine Delegate ==
 *   Copyright 2019 Karn Kaul
 * Features:
 *   - Header-only
 *   - Variadic template class providing `std::function<void(Args...)>` (any number of parameters)
 *   - Supports multiple callback registrants (thus `void` return type for each callback)
 *   - Token based, memory safe lifetime
 * Usage:
 *   - Create a `Delegate<>` for a simple `void()` callback, or `Delegate<Args...>` for passing arguments
 *   - Call `register()` on the object and store the received `Token` to receive the callback
 *   - Invoke the object (`foo()`) to fire all callbacks; returns number of active registrants
 *   - Discard the `Token` object to unregister a callback (recommend storing as a member variable for transient lifetime objects)
 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <functional>

namespace le
{
template <typename... Args>
class Delegate
{
public:
	using Token = std::shared_ptr<int32_t>;
	using Callback = std::function<void(Args... t)>;

private:
	using WToken = std::weak_ptr<int32_t>;

private:
	struct Wrapper
	{
		Callback callback;
		WToken wToken;
		Wrapper(Callback callback, Token token);
	};

	std::vector<Wrapper> m_callbacks;

public:
	// Returns shared_ptr to be owned by caller
	[[nodiscard]] Token subscribe(Callback callback);
	// Execute alive callbacks; returns live count
	uint32_t operator()(Args... t);
	// Returns true if any previously distributed Token is still alive
	bool isAlive();
	void clear();

private:
	// Remove expired weak_ptrs
	void Cleanup();
};

template <typename... Args>
Delegate<Args...>::Wrapper::Wrapper(Callback callback, Token token) : callback(std::move(callback)), wToken(token)
{
}

template <typename... Args>
typename Delegate<Args...>::Token Delegate<Args...>::subscribe(Callback callback)
{
	Token token = std::make_shared<int32_t>(int32_t(m_callbacks.size()));
	m_callbacks.emplace_back(std::move(callback), token);
	return token;
}

template <typename... Args>
uint32_t Delegate<Args...>::operator()(Args... t)
{
	Cleanup();
	for (auto const& c : m_callbacks)
	{
		c.callback(t...);
	}
	return static_cast<uint32_t>(m_callbacks.size());
}

template <typename... Args>
bool Delegate<Args...>::isAlive()
{
	Cleanup();
	return !m_callbacks.empty();
}

template <typename... Args>
void Delegate<Args...>::clear()
{
	m_callbacks.clear();
}

template <typename... Args>
void Delegate<Args...>::Cleanup()
{
	m_callbacks.erase(
		std::remove_if(m_callbacks.begin(), m_callbacks.end(), [](Wrapper& wrapper) -> bool { return wrapper.wToken.expired(); }),
		m_callbacks.end());
}
} // namespace le
