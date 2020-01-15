#include "le3d/core/gdata.hpp"
#include "le3d/gfx/bitmapFont.hpp"

namespace le
{
void FontAtlasData::deserialise(std::string json)
{
	GData data(std::move(json));
	cellSize = {data.getS32("cellX", cellSize.x), data.getS32("cellY", cellSize.y)};
	offset = {data.getS32("offsetX", offset.x), data.getS32("offsetY", offset.y)};
	colsRows = {data.getS32("cols", colsRows.x), data.getS32("rows", colsRows.y)};
	startCode = (u8)data.getS32("startCode", startCode);
}
} // namespace le
