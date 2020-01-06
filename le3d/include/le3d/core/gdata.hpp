#pragma once
#include <unordered_map>
#include <vector>
#include "le3d/stdtypes.hpp"

namespace le
{
// \brief Pseudo-JSON serialisable data container
class GData
{
private:
	std::unordered_map<std::string, std::string> m_fieldMap;

public:
	GData();
	// Pass serialised data to marhshall and load fields
	GData(std::string serialised);
	GData(GData const& rhs) = default;
	GData(GData&&) = default;
	GData& operator=(GData const&) = default;
	GData& operator=(GData&&) = default;
	~GData();

	// Marhshalls and load fields from serialised data
	bool marshall(std::string serialised);
	// Returns original raw data, without whitespaces and enclosing braces
	std::string unmarshall() const;
	// Clears raw data and fields
	void clear();

	std::string getStr(std::string const& key, std::string defaultValue = "") const;
	std::string getStr(std::string const& key, char spaceDelimiter, std::string defaultValue) const;
	bool getBool(std::string const& key, bool defaultValue = false) const;
	s32 getS32(std::string const& key, s32 defaultValue = -1) const;
	f64 getF64(std::string const& key, f64 defaultValue = -1.0) const;
	GData getGData(std::string const& key) const;

	std::vector<GData> getGDatas(std::string const& key) const;
	std::vector<std::string> getStrs(std::string const& key) const;

	std::unordered_map<std::string, std::string> const& allFields() const;
	bool addField(std::string key, GData& gData);
	bool setString(std::string key, std::string value);

	u32 fieldCount() const;
	bool contains(std::string const& id) const;
};
} // namespace le
