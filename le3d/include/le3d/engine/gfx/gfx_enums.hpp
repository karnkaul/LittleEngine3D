#pragma once
#include "le3d/core/std_types.hpp"
#include "le3d/core/flags.hpp"

namespace le
{
enum class DrawType : u8
{
	Dynamic = 0,
	Static,
	COUNT_
};

enum class TexType : u8
{
	Diffuse = 0,
	Specular,
	COUNT_
};

enum class TexWrap : u8
{
	Repeat = 0,
	ClampEdge,
	ClampBorder,
	COUNT_
};

enum class TexFilter : u8
{
	Linear = 0,
	Nearest,
	LinearMpLinear,
	LinearMpNearest,
	NearestMpLinear,
	NearestMpNearest,
	COUNT_
};

enum class ClearFlag : u8
{
	ColorBuffer = 0,
	DepthBuffer,
	StencilBuffer,
	COUNT_
};
using ClearFlags = TFlags<ClearFlag>;

enum class GLFlag : u8
{
	DepthTest = 0,
	Blend,
	COUNT_
};

enum class BlendFunc : u8
{
	Src_OneMinusSrc = 0,
	COUNT_
};

enum class StringProp : u8
{
	Renderer = 0,
	Vendor,
	Version,
	COUNT_
};

enum class PolygonFace : u8
{
	FrontAndBack,
	Front,
	Back,
	COUNT_
};

enum class PolygonMode : u8
{
	Fill = 0,
	Line,
	COUNT_
};

enum class GFXMode : u8
{
	BufferedThreaded = 0,
	BufferedMainThread,
	ImmediateMainThread,
	COUNT_
};
} // namespace le
