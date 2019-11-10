#pragma once 
#include < string>
#include "le3d/delegate.hpp"
#include "le3d/stdtypes.hpp"

namespace le
{
using OnText = Delegate<char>;

namespace input
{
std::string_view toStr(s32 key);
OnText::Token registerText(OnText::Callback callback);
}
} // namespace le
