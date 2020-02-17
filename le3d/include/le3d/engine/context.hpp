#pragma once
#include <filesystem>
#include <memory>
#include <glm/glm.hpp>
#include "le3d/core/colour.hpp"
#include "le3d/core/delegate.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/core/rect2.hpp"
#include "le3d/core/std_types.hpp"
#include "le3d/core/version.hpp"
#include "le3d/engine/gfx/gfx_enums.hpp"
#include "le3d/env/env.hpp"

namespace le
{
namespace context
{
using OnSwap = Delegate<>;

enum class WindowType : u8
{
	DecoratedWindow = 0,
	BorderlessWindow,
	BorderlessFullscreen,
	DedicatedFullscreen,
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
		WindowType type = WindowType::DecoratedWindow;
		u16 width = 1280;
		u16 height = 720;
		u16 screenID = 0;
	};
	struct ContextOpts
	{
		Version minVersion = Version(3, 3);
		GFXMode gfxMode = GFXMode::BufferedThreaded;
		bool bThreaded = false;
		bool bVSYNC = true;
	};

	WindowOpts window;
	LogOpts log;
	EnvOpts env;
	ContextOpts ctxt;
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
void pollEvents();
void setSwapInterval(u8 interval);
void swapAndPresent();

OnSwap::Token registerOnSwap(OnSwap::Callback callback);

bool setContextThread();
bool releaseContextThread();

u8 swapInterval();
u64 framesTicked();
u64 framesRendered();
f32 windowAspect();
glm::vec2 windowSize();
glm::vec2 project(glm::vec2 nPos, glm::vec2 space);
glm::vec2 projectScreen(glm::vec2 nPos);
glm::vec2 worldToScreen(glm::vec2 world);
} // namespace context
} // namespace le
