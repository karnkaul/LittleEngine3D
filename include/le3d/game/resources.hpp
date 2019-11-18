#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/shader.hpp"

namespace le
{
namespace shaders
{
Shader* loadShader(std::string id, std::string_view vertCode, std::string_view fragCode);
Shader* findShader(const std::string& id);
Shader& getShader(const std::string& id);

bool unload(Shader& shader);
void unloadAll();

u32 count();
} // namespace shaders
} // namespace le
