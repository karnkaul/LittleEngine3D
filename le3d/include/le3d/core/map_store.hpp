#pragma once
#include <map>
#include <unordered_map>

namespace le
{
template <typename MapContainer>
class TMapStore final
{
public:
	using Key = typename MapContainer::key_type;
	using Value = typename MapContainer::mapped_type;

private:
	MapContainer m_map;
	Value m_nullValue;

public:
	explicit TMapStore(Value nullValue = {});

public:
	Value const& get(Key const& id) const;
	Value& get(Key const& id);
	bool isLoaded(Key const& id) const;
	bool unload(Key const& id);
	void unloadAll();
	u32 count() const;
	auto& getMap();
	Value& getNull();
};

template <typename MapContainer>
TMapStore<MapContainer>::TMapStore(Value nullValue) : m_nullValue(std::move(nullValue))
{
}

template <typename MapContainer>
typename TMapStore<MapContainer>::Value const& TMapStore<MapContainer>::get(Key const& id) const
{
	auto search = m_map.find(id);
	if (search != m_map.end())
	{
		return search->second;
	}
	return m_nullValue;
}

template <typename MapContainer>
typename TMapStore<MapContainer>::Value& TMapStore<MapContainer>::get(Key const& id)
{
	auto search = m_map.find(id);
	if (search != m_map.end())
	{
		return search->second;
	}
	return m_nullValue;
}

template <typename MapContainer>
bool TMapStore<MapContainer>::isLoaded(Key const& id) const
{
	return m_map.find(id) != m_map.end();
}

template <typename MapContainer>
bool TMapStore<MapContainer>::unload(Key const& id)
{
	auto search = m_map.find(id);
	if (search != m_map.end())
	{
		m_map.erase(search);
		return true;
	}
	return false;
}

template <typename MapContainer>
void TMapStore<MapContainer>::unloadAll()
{
	m_map.clear();
	return;
}

template <typename MapContainer>
u32 TMapStore<MapContainer>::count() const
{
	return (u32)m_map.size();
}

template <typename MapContainer>
auto& TMapStore<MapContainer>::getMap()
{
	return m_map;
}

template <typename MapContainer>
typename TMapStore<MapContainer>::Value& TMapStore<MapContainer>::getNull()
{
	return m_nullValue;
}
} // namespace le
