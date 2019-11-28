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
	GData(const GData& rhs) = default;
	GData(GData&&) = default;
	GData& operator=(const GData&) = default;
	GData& operator=(GData&&) = default;
	~GData();

	// Marhshalls and load fields from serialised data
	bool marshall(std::string serialised);
	// Returns original raw data, without whitespaces and enclosing braces
	std::string unmarshall() const;
	// Clears raw data and fields
	void clear();

	std::string getString(const std::string& key, std::string defaultValue = "") const;
	std::string getString(const std::string& key, char spaceDelimiter, std::string defaultValue) const;
	bool getBool(const std::string& key, bool defaultValue = false) const;
	s32 getS32(const std::string& key, s32 defaultValue = -1) const;
	f64 getF64(const std::string& key, f64 defaultValue = -1.0) const;
	GData getGData(const std::string& key) const;

	std::vector<GData> getGDatas(const std::string& key) const;
	std::vector<std::string> getStrings(const std::string& key) const;

	const std::unordered_map<std::string, std::string>& allFields() const;
	bool addField(std::string key, GData& gData);
	bool setString(std::string key, std::string value);

	u32 fieldCount() const;
	bool contains(const std::string& id) const;
};
} // namespace le
