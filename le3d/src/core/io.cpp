#include <fstream>
#include <physfs/physfs.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/io.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "ioImpl.hpp"

namespace le
{
namespace
{
struct PhysfsHandle final
{
	PhysfsHandle();
	~PhysfsHandle();
};

PhysfsHandle::PhysfsHandle()
{
	PHYSFS_init(env::argv0().data());
}

PhysfsHandle::~PhysfsHandle()
{
	PHYSFS_deinit();
}

std::unique_ptr<PhysfsHandle> g_uPhysfsHandle;
} // namespace

bytearray IOReader::FBytes::operator()(stdfs::path const& id) const
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
		LOG_E("!! [%s] not found in %s!", id.generic_string().data(), m_medium.data());
		return false;
	}
	return true;
}

FileReader::FileReader(stdfs::path prefix) : IOReader(std::move(prefix))
{
	m_medium = "Filesystem (";
	m_medium += std::move(m_prefix.generic_string());
	m_medium += ")";
	LOG_D("[%s] Filesystem mounted, idPrefix: [%s]", Typename(*this).data(), m_prefix.generic_string().data());
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

bytearray FileReader::getBytes(stdfs::path const& id) const
{
	bytearray buf;
	if (checkPresence(id))
	{
		std::ifstream file(m_prefix / id, std::ios::binary | std::ios::ate);
		if (file.good())
		{
			auto pos = file.tellg();
			buf = bytearray((size_t)pos);
			file.seekg(0, std::ios::beg);
			file.read((char*)buf.data(), pos);
		}
	}
	return buf;
}

ZIPReader::ZIPReader(stdfs::path zipPath, stdfs::path idPrefix /* = "" */) : IOReader(std::move(idPrefix)), m_zipPath(std::move(zipPath))
{
	ioImpl::initPhysfs();
	m_medium = "ZIP (";
	m_medium += std::move(m_zipPath.generic_string());
	m_medium += ")";
	if (!stdfs::is_regular_file(m_zipPath))
	{
		LOG_E("[%s] [%s] not found on Filesystem!", Typename<ZIPReader>().data(), m_zipPath.generic_string().data());
	}
	else
	{
		PHYSFS_mount(m_zipPath.string().data(), nullptr, 0);
		LOG_D("[%s] [%s] archive mounted, idPrefix: [%s]", Typename(*this).data(), m_zipPath.generic_string().data(),
			  m_prefix.generic_string().data());
	}
}

bool ZIPReader::isPresent(stdfs::path const& id) const
{
	return PHYSFS_exists((m_prefix / id).generic_string().data()) != 0;
}

std::stringstream ZIPReader::getStr(stdfs::path const& id) const
{
	std::stringstream buf;
	if (checkPresence(id))
	{
		auto pFile = PHYSFS_openRead((m_prefix / id).generic_string().data());
		if (pFile)
		{
			auto length = PHYSFS_fileLength(pFile);
			std::string charBuf((size_t)length, 0);
			PHYSFS_readBytes(pFile, charBuf.data(), (PHYSFS_uint64)length);
			buf << charBuf;
		}
		PHYSFS_close(pFile);
	}
	return buf;
}

bytearray ZIPReader::getBytes(stdfs::path const& id) const
{
	bytearray buf;
	if (checkPresence(id))
	{
		auto pFile = PHYSFS_openRead((m_prefix / id).generic_string().data());
		if (pFile)
		{
			auto length = PHYSFS_fileLength(pFile);
			buf = bytearray((size_t)length);
			PHYSFS_readBytes(pFile, buf.data(), (PHYSFS_uint64)length);
		}
		PHYSFS_close(pFile);
	}
	return buf;
}

void ioImpl::initPhysfs()
{
	if (!g_uPhysfsHandle)
	{
		g_uPhysfsHandle = std::make_unique<PhysfsHandle>();
		LOG_D("PhysFS initialised");
	}
}

void ioImpl::deinitPhysfs()
{
	LOG_D("PhysFS deinitialised");
	g_uPhysfsHandle = nullptr;
}
} // namespace le
