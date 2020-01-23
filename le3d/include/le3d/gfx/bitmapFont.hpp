#pragma once
#include "mesh.hpp"

namespace le
{
struct FontAtlasData final
{
	glm::ivec2 cellSize = glm::ivec2(0);
	glm::ivec2 colsRows = glm::ivec2(0);
	glm::ivec2 offset = glm::ivec2(0);
	bytearray bytes;
	std::string samplerID;
	u8 startCode = 32;

	void deserialise(std::string json);
};

class BitmapFont final
{
public:
	Mesh quad;
	std::string id;
	HTexture sheet;
	glm::ivec2 cellSize = glm::ivec2(0);
	glm::ivec2 colsRows = glm::ivec2(0);
	glm::ivec2 offset = glm::ivec2(0);
	u8 startCode = 0;
};
} // namespace le
