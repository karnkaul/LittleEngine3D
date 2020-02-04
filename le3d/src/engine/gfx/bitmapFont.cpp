#include "le3d/core/gdata.hpp"
#include "le3d/engine/gfx/bitmapFont.hpp"

namespace le
{
void FontAtlasData::deserialise(std::string json)
{
	GData data(std::move(json));
	cellSize = {data.get<s32>("cellX", cellSize.x), data.get<s32>("cellY", cellSize.y)};
	offset = {data.get<s32>("offsetX", offset.x), data.get<s32>("offsetY", offset.y)};
	colsRows = {data.get<s32>("cols", colsRows.x), data.get<s32>("rows", colsRows.y)};
	startCode = (u8)data.get<s32>("startCode", startCode);
	samplerID = data.get<std::string>("sampler", "font");
	return;
}
} // namespace le
