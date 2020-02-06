#pragma once
#include <functional>
#include "le3d/core/colour.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/core/io.hpp"

namespace le::manifestLoader
{
struct Manifest
{
	stdfs::path id;
	IOReader const* pReader = nullptr;
};

struct Args
{
	Time dt;
	f32 progress;
};

struct Request
{
	std::function<void(Args)> doFrame;
	Manifest manifest;
	Colour clearColour;
	u16 extraSwaps = 1;
};

void load(Request request);
void unload(Manifest manifest);
} // namespace le::manifestLoader
