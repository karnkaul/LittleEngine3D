#pragma once
#include <filesystem>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/env/env.hpp"

namespace le
{
namespace context
{
enum class Type
{
	BorderedWindow,
	BorderlessFullscreen,
	Dedicated,
};

struct Settings
{
	struct LogOpts
	{
		std::filesystem::path filename = "debug.log";
		env::Dir dir = env::Dir::Working;
		bool bLogToFile = true;
	};
	struct EnvOpts
	{
		env::Args args;
		u16 jobWorkerCount = 2;
	};

	LogOpts log;
	EnvOpts env;
	std::string title = "LittleEngine3D";
	Type type = Type::BorderedWindow;
	u16 width = 1280;
	u16 height = 720;
	u16 screenID = 0;
	bool bVSYNC = true;
};

struct Wrapper final
{
private:
	const bool m_bValid = false;

public:
	Wrapper(bool bValid = false);
	Wrapper(const Wrapper&) = delete;
	Wrapper& operator=(const Wrapper&) = delete;
	~Wrapper();

	operator bool() const;
};

Wrapper create(const Settings& pSettings);
void destroy();

bool exists();
bool isClosing();
void clearFlags(u32 flags, Colour colour = Colour::Black);
void pollEvents();
void swapBuffers();

f32 nativeAR();
glm::vec2 size();
glm::vec2 project(glm::vec2 nPos, glm::vec2 space);
glm::vec2 projectScreen(glm::vec2 nPos);
glm::vec2 worldToScreen(glm::vec2 world);
} // namespace context
} // namespace le
