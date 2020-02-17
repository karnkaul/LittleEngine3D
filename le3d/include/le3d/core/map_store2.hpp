#pragma once
#include <map>
#include <mutex>
#include <unordered_map>
#include "std_types.hpp"

namespace le
{
template <typename MapContainer>
class TMapStore2 final
{
public:
	using Key = typename MapContainer::key_type;
	using Value = typename MapContainer::mapped_type;

private:
	using Lock = std::lock_guard<std::mutex>;

private:
	MapContainer m_map;
	mutable std::mutex m_mutex;

public:
	void insert(Key const& id, Value&& value);
	Value const* get(Key const& id) const;
	Value* get(Key const& id);
	bool isLoaded(Key const& id) const;
	bool unload(Key const& id);
	void unloadAll();
	u32 count() const;
};

template <typename MapContainer>
void TMapStore2<MapContainer>::insert(Key const& id, Value&& value)
{
	Lock lock(m_mutex);
	m_map[id] = std::move(value);
}

template <typename MapContainer>
typename TMapStore2<MapContainer>::Value const* TMapStore2<MapContainer>::get(Key const& id) const
{
	Lock lock(m_mutex);
	auto search = m_map.find(id);
	return search != m_map.end() ? &search->second : nullptr;
}

template <typename MapContainer>
typename TMapStore2<MapContainer>::Value* TMapStore2<MapContainer>::get(Key const& id)
{
	Lock lock(m_mutex);
	auto search = m_map.find(id);
	return search != m_map.end() ? &search->second : nullptr;
}

template <typename MapContainer>
bool TMapStore2<MapContainer>::isLoaded(Key const& id) const
{
	Lock lock(m_mutex);
	return m_map.find(id) != m_map.end();
}

template <typename MapContainer>
bool TMapStore2<MapContainer>::unload(Key const& id)
{
	Lock lock(m_mutex);
	auto search = m_map.find(id);
	if (search != m_map.end())
	{
		m_map.erase(search);
		return true;
	}
	return false;
}

template <typename MapContainer>
void TMapStore2<MapContainer>::unloadAll()
{
	Lock lock(m_mutex);
	m_map.clear();
	return;
}

template <typename MapContainer>
u32 TMapStore2<MapContainer>::count() const
{
	Lock lock(m_mutex);
	return (u32)m_map.size();
}
} // namespace le
