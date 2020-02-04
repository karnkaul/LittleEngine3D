#pragma once
#include <unordered_map>
#include <vector>
#include "le3d/core/stdtypes.hpp"

namespace le
{
// \brief Pseudo-JSON serialisable data container
class GData
{
protected:
	std::unordered_map<std::string, std::string> m_fieldMap;

public:
	GData() noexcept;
	// Pass serialised data to marhshall and load fields
	explicit GData(std::string serialised);
	GData(GData&&) noexcept;
	GData& operator=(GData&&) noexcept;
	GData(GData const& rhs);
	GData& operator=(GData const&);
	virtual ~GData();

	// Marhshalls and load fields from serialised data
	bool marshall(std::string serialised);
	// Returns original raw data, without whitespaces and enclosing braces
	std::string unmarshall() const;
	// Clears raw data and fields
	void clear();

	template <typename Type>
	Type get(std::string const& key, Type defaultValue = Type()) const;

	template <>
	std::string get(std::string const& key, std::string defaultValue) const;
	template <>
	bool get(std::string const& key, bool defaultValue) const;
	template <>
	s32 get(std::string const& key, s32 defaultValue) const;
	template <>
	f64 get(std::string const& key, f64 defaultValue) const;
	template <>
	std::vector<std::string> get(std::string const& key, std::vector<std::string> defaultValue) const;

	GData getGData(std::string const& key) const;
	std::vector<GData> getGDatas(std::string const& key) const;

	std::unordered_map<std::string, std::string> const& allFields() const;
	bool addField(std::string key, GData& gData);
	bool setString(std::string key, std::string value);

	u32 fieldCount() const;
	bool contains(std::string const& id) const;
};

template <typename Type>
Type GData::get(std::string const&, Type) const
{
	static_assert(alwaysFalse<Type>, "Invalid type!");
}
} // namespace le
