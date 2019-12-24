#include "le3d/core/gdata.hpp"
#include "le3d/core/utils.hpp"

namespace le
{
namespace
{
std::initializer_list<std::pair<char, char>> gDataEscapes = {{'{', '}'}, {'[', ']'}, {'"', '"'}};

template <typename T>
T Get(const std::unordered_map<std::string, std::string>& table, const std::string& key, T (*Adaptor)(std::string, T),
	  const T& defaultValue)
{
	auto search = table.find(key);
	if (search != table.end())
	{
		return Adaptor(search->second, defaultValue);
	}
	return defaultValue;
}
} // namespace

GData::GData(std::string serialised)
{
	if (!serialised.empty())
	{
		marshall(std::move(serialised));
	}
}

GData::GData() = default;
GData::~GData() = default;

bool GData::marshall(std::string serialised)
{
	le::utils::strings::removeChars(serialised, {'\t', '\r', '\n'});
	le::utils::strings::trim(serialised, {' '});
	if (serialised[0] == '{' && serialised[serialised.size() - 1] == '}')
	{
		clear();
		std::string rawText = serialised.substr(1, serialised.size() - 2);
		std::vector<std::string> tokens = le::utils::strings::tokenise(rawText, ',', gDataEscapes);
		for (const auto& token : tokens)
		{
			std::pair<std::string, std::string> kvp = le::utils::strings::bisect(token, ':');
			if (!kvp.second.empty() && !kvp.first.empty())
			{
				std::initializer_list<char> trim = {' ', '"'};
				le::utils::strings::trim(kvp.first, trim);
				le::utils::strings::trim(kvp.second, trim);
				m_fieldMap.emplace(std::move(kvp.first), std::move(kvp.second));
			}
		}
		return true;
	}

	return false;
}

std::string GData::unmarshall() const
{
	std::string ret = "{";
	size_t slice = 0;
	for (const auto& kvp : m_fieldMap)
	{
		std::string value = kvp.second;
		le::utils::strings::trim(value, {' '});
		auto space = value.find(' ');
		if (le::utils::strings::isCharEnclosedIn(value, space, {'"', '"'}))
		{
			value = '\"' + value + '\"';
		}
		ret += (kvp.first + ':' + value + ',');
		slice = 1;
	}
	return ret.substr(0, ret.size() - slice) + '}';
}

void GData::clear()
{
	m_fieldMap.clear();
}

std::string GData::getString(const std::string& key, std::string defaultValue) const
{
	auto search = m_fieldMap.find(key);
	if (search != m_fieldMap.end())
	{
		return search->second;
	}
	return defaultValue;
}

std::string GData::getString(const std::string& key, char spaceDelimiter, std::string defaultValue) const
{
	std::string ret = std::move(defaultValue);
	auto search = m_fieldMap.find(key);
	if (search != m_fieldMap.end())
	{
		ret = search->second;
		le::utils::strings::substituteChars(ret, {std::pair<char, char>(spaceDelimiter, ' ')});
	}
	return ret;
}

bool GData::getBool(const std::string& key, bool defaultValue) const
{
	return Get<bool>(m_fieldMap, key, &le::utils::strings::toBool, defaultValue);
}

s32 GData::getS32(const std::string& key, s32 defaultValue) const
{
	return Get<s32>(m_fieldMap, key, &le::utils::strings::toS32, defaultValue);
}

f64 GData::getF64(const std::string& key, f64 defaultValue) const
{
	return Get<f64>(m_fieldMap, key, &le::utils::strings::toF64, defaultValue);
}

GData GData::getGData(const std::string& key) const
{
	auto search = m_fieldMap.find(key);
	if (search != m_fieldMap.end())
	{
		return GData(search->second);
	}
	return {};
}

std::vector<GData> GData::getGDatas(const std::string& key) const
{
	std::vector<GData> ret;
	std::vector<std::string> rawStrings = getStrings(key);
	for (auto& rawString : rawStrings)
	{
		le::utils::strings::trim(rawString, {'"', ' '});
		ret.emplace_back(std::move(rawString));
	}
	return ret;
}

std::vector<std::string> GData::getStrings(const std::string& key) const
{
	auto search = m_fieldMap.find(key);
	if (search != m_fieldMap.end())
	{
		std::string value = search->second;
		if (value.size() > 2)
		{
			le::utils::strings::trim(value, {' '});
			if (value[0] == '[' && value[value.size() - 1] == ']')
			{
				value = value.substr(1, value.size() - 2);
				le::utils::strings::trim(value, {' '});
				std::vector<std::string> ret = le::utils::strings::tokenise(value, ',', gDataEscapes);
				for (auto& str : ret)
				{
					le::utils::strings::trim(str, {'"', ' '});
				}
				return ret;
			}
		}
	}
	return std::vector<std::string>();
}

const std::unordered_map<std::string, std::string>& GData::allFields() const
{
	return m_fieldMap;
}

bool GData::addField(std::string key, GData& gData)
{
	std::string value = gData.unmarshall();
	le::utils::strings::removeChars(value, {'\"'});
	return setString(std::move(key), std::move(value));
}

bool GData::setString(std::string key, std::string value)
{
	if (!key.empty() && !value.empty())
	{
		m_fieldMap[std::move(key)] = std::move(value);
		return true;
	}
	return false;
}

u32 GData::fieldCount() const
{
	return static_cast<u32>(m_fieldMap.size());
}

bool GData::contains(const std::string& id) const
{
	return m_fieldMap.find(id) != m_fieldMap.end();
}
} // namespace le
