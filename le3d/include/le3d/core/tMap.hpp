#pragma once
#include <map>
#include <unordered_map>

namespace le
{
template <typename MapContainer>
class TMap final
{
public:
	using Key = typename MapContainer::key_type;
	using Value = typename MapContainer::mapped_type;

private:
	MapContainer m_map;
	Value m_nullValue;

public:
	explicit TMap(Value nullValue = {});

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
TMap<MapContainer>::TMap(Value nullValue) : m_nullValue(std::move(nullValue))
{
}

template <typename MapContainer>
typename TMap<MapContainer>::Value const& TMap<MapContainer>::get(Key const& id) const
{
	auto search = m_map.find(id);
	if (search != m_map.end())
	{
		return search->second;
	}
	return m_nullValue;
}

template <typename MapContainer>
typename TMap<MapContainer>::Value& TMap<MapContainer>::get(Key const& id)
{
	auto search = m_map.find(id);
	if (search != m_map.end())
	{
		return search->second;
	}
	return m_nullValue;
}

template <typename MapContainer>
bool TMap<MapContainer>::isLoaded(Key const& id) const
{
	return m_map.find(id) != m_map.end();
}

template <typename MapContainer>
bool TMap<MapContainer>::unload(Key const& id)
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
void TMap<MapContainer>::unloadAll()
{
	m_map.clear();
}

template <typename MapContainer>
u32 TMap<MapContainer>::count() const
{
	return (u32)m_map.size();
}

template <typename MapContainer>
auto& TMap<MapContainer>::getMap()
{
	return m_map;
}

template <typename MapContainer>
typename TMap<MapContainer>::Value& TMap<MapContainer>::getNull()
{
	return m_nullValue;
}
} // namespace le
