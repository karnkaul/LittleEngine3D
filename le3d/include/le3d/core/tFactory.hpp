#pragma once
#include <memory>
#include <type_traits>
#include <unordered_map>
#include "le3d/stdtypes.hpp"

namespace le
{
template <typename FType, typename FHandle = s32>
class Factory
{
public:
	std::unordered_map<FHandle, FType> m_instanced;

protected:
	FHandle m_nextHandle = FHandle(1);

public:
	Factory();
	Factory(Factory&&) noexcept;
	Factory& operator=(Factory&&) noexcept;
	virtual ~Factory();

public:
	template <typename T>
	FHandle construct();
	template <typename T>
	T* find(FHandle h);
	bool destroy(FHandle h);
	void clear();
};

template <typename FType, typename FHandle = s32>
class UFactory
{
public:
	std::unordered_map<FHandle, std::unique_ptr<FType>> m_instanced;

protected:
	FHandle m_nextHandle = FHandle(1);

public:
	UFactory();
	UFactory(UFactory&&) noexcept;
	UFactory& operator=(UFactory&&) noexcept;
	virtual ~UFactory();

public:
	template <typename T>
	FHandle construct();
	template <typename T>
	T* find(FHandle h) const;
	bool destroy(FHandle h);
	void clear();
};

template <typename FType, typename FHandle>
Factory<FType, FHandle>::Factory() = default;

template <typename FType, typename FHandle>
Factory<FType, FHandle>::Factory(Factory&&) noexcept = default;

template <typename FType, typename FHandle>
Factory<FType, FHandle>& Factory<FType, FHandle>::operator=(Factory&&) noexcept = default;

template <typename FType, typename FHandle>
Factory<FType, FHandle>::~Factory()
{
	m_instanced.clear();
}

template <typename FType, typename FHandle>
template <typename T>
FHandle Factory<FType, FHandle>::construct()
{
	static_assert(std::is_base_of<FType, T>::value, "T must derive from FType!");
	T t;
	m_instanced.emplace(m_nextHandle, std::move(t));
	return m_nextHandle++;
}

template <typename FType, typename FHandle>
template <typename T>
T* Factory<FType, FHandle>::find(FHandle h)
{
	static_assert(std::is_base_of<FType, T>::value, "T must derive from FType!");
	auto search = m_instanced.find(h);
	if (search != m_instanced.end())
	{
		return dynamic_cast<T*>(&search->second);
	}
	return nullptr;
}

template <typename FType, typename FHandle>
bool Factory<FType, FHandle>::destroy(FHandle h)
{
	auto search = m_instanced.find(h);
	if (search != m_instanced.end())
	{
		m_instanced.erase(search);
		return true;
	}
	return false;
}

template <typename FType, typename FHandle>
void Factory<FType, FHandle>::clear()
{
	m_instanced.clear();
}

template <typename FType, typename FHandle>
UFactory<FType, FHandle>::UFactory() = default;

template <typename FType, typename FHandle>
UFactory<FType, FHandle>::UFactory(UFactory&&) noexcept = default;

template <typename FType, typename FHandle>
UFactory<FType, FHandle>& UFactory<FType, FHandle>::operator=(UFactory&&) noexcept = default;

template <typename FType, typename FHandle>
UFactory<FType, FHandle>::~UFactory()
{
	m_instanced.clear();
}

template <typename FType, typename FHandle>
template <typename T>
FHandle UFactory<FType, FHandle>::construct()
{
	static_assert(std::is_base_of<FType, T>::value, "T must derive from FType!");
	auto uT = std::make_unique<T>();
	m_instanced.emplace(m_nextHandle, std::move(uT));
	return m_nextHandle++;
}

template <typename FType, typename FHandle>
template <typename T>
T* UFactory<FType, FHandle>::find(FHandle h) const
{
	static_assert(std::is_base_of<FType, T>::value, "T must derive from FType!");
	auto search = m_instanced.find(h);
	if (search != m_instanced.end())
	{
		return dynamic_cast<T*>(search->second.get());
	}
	return nullptr;
}

template <typename FType, typename FHandle>
bool UFactory<FType, FHandle>::destroy(FHandle h)
{
	auto search = m_instanced.find(h);
	if (search != m_instanced.end())
	{
		m_instanced.erase(search);
		return true;
	}
	return false;
}

template <typename FType, typename FHandle>
void UFactory<FType, FHandle>::clear()
{
	m_instanced.clear();
}
} // namespace le
