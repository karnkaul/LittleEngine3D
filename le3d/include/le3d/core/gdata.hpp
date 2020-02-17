#pragma once
#include <unordered_map>
#include <vector>
#include "le3d/core/std_types.hpp"

namespace le
{
// \brief Pseudo-JSON serialisable data container
class GData
{
protected:
	std::unordered_map<std::string, std::string> m_fieldMap;

public:
	GData();
	// Pass serialised data to marhshall and load fields
	explicit GData(std::string serialised);
	GData(GData&&);
	GData& operator=(GData&&);
	GData(GData const& rhs);
	GData& operator=(GData const&);
	virtual ~GData();

	// Marhshalls and load fields from serialised data
	bool marshall(std::string serialised);
	// Returns original raw data, without whitespaces and enclosing braces
	std::string unmarshall() const;
	// Clears raw data and fields
	void clear();

	std::string getString(std::string const& key, std::string defaultValue = "") const;
	bool getBool(std::string const& key, bool defaultValue) const;
	s32 getS32(std::string const& key, s32 defaultValue) const;
	f64 getF64(std::string const& key, f64 defaultValue) const;
	std::vector<std::string> getVecString(std::string const& key) const;

	GData getGData(std::string const& key) const;
	std::vector<GData> getGDatas(std::string const& key) const;

	std::unordered_map<std::string, std::string> const& allFields() const;
	bool addField(std::string key, GData& gData);
	bool setString(std::string key, std::string value);

	u32 fieldCount() const;
	bool contains(std::string const& id) const;
};
} // namespace le
