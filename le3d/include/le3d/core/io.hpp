#pragma once
#include <filesystem>
#include <sstream>
#include "le3d/core/stdtypes.hpp"

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
	IOReader(stdfs::path prefix);
	IOReader(IOReader&&);
	IOReader& operator=(IOReader&&);
	IOReader(IOReader const&);
	IOReader& operator=(IOReader const&);
	virtual ~IOReader();

public:
	std::string getString(stdfs::path const& id) const;
	FBytes bytesFunctor() const;
	FStr strFunctor() const;
	std::string_view medium() const;
	bool checkPresence(stdfs::path const& id) const;

public:
	virtual bool isPresent(stdfs::path const& id) const = 0;
	virtual bytearray getBytes(stdfs::path const& id) const = 0;
	virtual std::stringstream getStr(stdfs::path const& id) const = 0;
};

class FileReader : public IOReader
{
public:
	FileReader(stdfs::path prefix = "");

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
