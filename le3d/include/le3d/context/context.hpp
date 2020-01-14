#pragma once
#include <filesystem>
#include <memory>
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
	struct WindowOpts
	{
		std::string title = "LittleEngine3D";
		Type type = Type::BorderedWindow;
		u16 width = 1280;
		u16 height = 720;
		u16 screenID = 0;
		bool bVSYNC = true;
	};

	WindowOpts window;
	LogOpts log;
	EnvOpts env;
	// Invokes `threads::joinAll()` on context destruction
	bool bJoinThreadsOnDestroy = true;
};

struct HContext final
{
	~HContext();
};

std::unique_ptr<HContext> create(Settings const& settings);
bool isAlive();
void close();
bool isClosing();
void clearFlags(u32 flags, Colour colour = Colour::Black);
void pollEvents();
void swapBuffers();

u64 swapCount();
f32 nativeAR();
glm::vec2 size();
glm::vec2 project(glm::vec2 nPos, glm::vec2 space);
glm::vec2 projectScreen(glm::vec2 nPos);
glm::vec2 worldToScreen(glm::vec2 world);
} // namespace context
} // namespace le
