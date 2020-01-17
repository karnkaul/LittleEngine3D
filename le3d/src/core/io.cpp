#include <fstream>
#include "le3d/core/assert.hpp"
#include "le3d/core/io.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"

namespace le
{
bytestream IOReader::FBytes::operator()(stdfs::path const& id) const
{
	return pReader->getBytes(pReader->m_prefix / id);
}

std::stringstream IOReader::FStr::operator()(stdfs::path const& id) const
{
	return pReader->getStr(pReader->m_prefix / id);
}

IOReader::IOReader(stdfs::path prefix) : m_prefix(std::move(prefix)), m_medium("Undefined")
{
	m_getBytes.pReader = this;
	m_getStr.pReader = this;
}

IOReader::IOReader(IOReader&&) = default;
IOReader& IOReader::operator=(IOReader&&) = default;
IOReader::IOReader(IOReader const&) = default;
IOReader& IOReader::operator=(IOReader const&) = default;
IOReader::~IOReader() = default;

std::string IOReader::getString(stdfs::path const& id) const
{
	return getStr(id).str();
}

IOReader::FBytes IOReader::bytesFunctor() const
{
	return m_getBytes;
}

IOReader::FStr IOReader::strFunctor() const
{
	return m_getStr;
}

std::string_view IOReader::medium() const
{
	return m_medium;
}

bool IOReader::checkPresence(stdfs::path const& id) const
{
	if (!isPresent(id))
	{
		LOG_E("!! [%s] not present on [%s (%s)]!", id.generic_string().data(), m_medium.data(), m_prefix.generic_string().data());
		return false;
	}
	return true;
}

FileReader::FileReader(stdfs::path prefix) : IOReader(std::move(prefix))
{
	m_medium = "Filesystem";
}

bool FileReader::isPresent(stdfs::path const& id) const
{
	return stdfs::is_regular_file(m_prefix / id);
}

std::stringstream FileReader::getStr(stdfs::path const& id) const
{
	std::stringstream buf;
	if (checkPresence(id))
	{
		std::ifstream file(m_prefix / id);
		if (file.good())
		{
			buf << file.rdbuf();
		}
	}
	return buf;
}

bytestream FileReader::getBytes(stdfs::path const& id) const
{
	bytestream buf;
	if (checkPresence(id))
	{
		std::ifstream file(m_prefix / id, std::ios::binary);
		if (file.good())
		{
			buf = std::vector<u8>(std::istreambuf_iterator<char>(file), {});
		}
	}
	return buf;
}
} // namespace le
