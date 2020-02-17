#pragma once
#include <filesystem>
#include <sstream>
#include "le3d/core/std_types.hpp"

namespace le
{
namespace stdfs = std::filesystem;

class IOReader
{
public:
	struct FBytes
	{
	private:
		IOReader const* pReader;

	public:
		bytearray operator()(stdfs::path const& id) const;

	private:
		friend class IOReader;
	};

	struct FStr
	{
	private:
		IOReader const* pReader;

	public:
		std::stringstream operator()(stdfs::path const& id) const;

	private:
		friend class IOReader;
	};

protected:
	stdfs::path m_prefix;
	std::string m_medium;
	FBytes m_getBytes;
	FStr m_getStr;

public:
	explicit IOReader(stdfs::path prefix) noexcept;
	IOReader(IOReader&&) noexcept;
	IOReader& operator=(IOReader&&) noexcept;
	IOReader(IOReader const&);
	IOReader& operator=(IOReader const&);
	virtual ~IOReader();

public:
	[[nodiscard]] std::string getString(stdfs::path const& id) const;
	[[nodiscard]] FBytes bytesFunctor() const;
	[[nodiscard]] FStr strFunctor() const;
	std::string_view medium() const;
	[[nodiscard]] bool checkPresence(stdfs::path const& id) const;
	[[nodiscard]] bool checkPresence(std::initializer_list<stdfs::path> ids) const;

public:
	[[nodiscard]] virtual bool isPresent(stdfs::path const& id) const = 0;
	[[nodiscard]] virtual bytearray getBytes(stdfs::path const& id) const = 0;
	[[nodiscard]] virtual std::stringstream getStr(stdfs::path const& id) const = 0;
};

class FileReader : public IOReader
{
public:
	FileReader(stdfs::path prefix = "") noexcept;

public:
	bool isPresent(stdfs::path const& id) const override;
	bytearray getBytes(stdfs::path const& id) const override;
	std::stringstream getStr(stdfs::path const& id) const override;
};

class ZIPReader : public IOReader
{
protected:
	stdfs::path m_zipPath;

public:
	ZIPReader(stdfs::path zipPath, stdfs::path idPrefix = "");

public:
	bool isPresent(stdfs::path const& id) const override;
	bytearray getBytes(stdfs::path const& id) const override;
	std::stringstream getStr(stdfs::path const& id) const override;
};
} // namespace le
