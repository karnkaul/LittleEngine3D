#include <algorithm>
#include <array>
#include <stack>
#include "le3d/core/utils.hpp"

namespace le
{
std::pair<f32, std::string_view> utils::friendlySize(u64 byteCount)
{
	static std::array suffixes = {"B", "KiB", "MiB", "GiB"};
	f32 bytes = f32(byteCount);
	size_t idx = 0;
	while (bytes > 1024.0f && idx < 4)
	{
		++idx;
		bytes /= 1024.0f;
	}
	return std::make_pair(bytes, suffixes[idx < 4 ? idx : 3]);
}

namespace utils::strings
{
void toLower(std::string& outString)
{
	std::transform(outString.begin(), outString.end(), outString.begin(), ::tolower);
	return;
}

void toUpper(std::string& outString)
{
	std::transform(outString.begin(), outString.end(), outString.begin(), ::toupper);
	return;
}

bool toBool(std::string input, bool bDefaultValue)
{
	if (!input.empty())
	{
		toLower(input);
		if (input == "true" || input == "1")
		{
			return true;
		}
		if (input == "false" || input == "0")
		{
			return false;
		}
	}
	return bDefaultValue;
}

s32 toS32(std::string input, s32 defaultValue)
{
	s32 ret = defaultValue;
	if (!input.empty())
	{
		try
		{
			ret = std::stoi(input);
		}
		catch (std::invalid_argument const&)
		{
			ret = defaultValue;
		}
	}
	return ret;
}

f32 toF32(std::string input, f32 defaultValue)
{
	f32 ret = defaultValue;
	if (!input.empty())
	{
		try
		{
			ret = std::stof(input);
		}
		catch (std::invalid_argument const&)
		{
			ret = defaultValue;
		}
	}
	return ret;
}

f64 toF64(std::string input, f64 defaultValue)
{
	f64 ret = defaultValue;
	if (!input.empty())
	{
		try
		{
			ret = std::stod(input);
		}
		catch (std::invalid_argument const&)
		{
			ret = defaultValue;
		}
	}
	return ret;
}

std::string toText(bytearray rawBuffer)
{
	std::vector<char> charBuffer(rawBuffer.size() + 1, 0);
	for (size_t i = 0; i < rawBuffer.size(); ++i)
	{
		charBuffer[i] = static_cast<char>(rawBuffer[i]);
	}
	return std::string(charBuffer.data());
}

std::pair<std::string, std::string> bisect(std::string_view input, char delimiter)
{
	size_t idx = input.find(delimiter);
	return idx < input.size() ? std::pair<std::string, std::string>(input.substr(0, idx), input.substr(idx + 1, input.size()))
							  : std::pair<std::string, std::string>(std::string(input), {});
}

void removeChars(std::string& outInput, std::initializer_list<char> toRemove)
{
	auto isToRemove = [&toRemove](char c) -> bool { return std::find(toRemove.begin(), toRemove.end(), c) != toRemove.end(); };
	auto iter = std::remove_if(outInput.begin(), outInput.end(), isToRemove);
	outInput.erase(iter, outInput.end());
	return;
}

void trim(std::string& outInput, std::initializer_list<char> toRemove)
{
	auto isIgnored = [&outInput, &toRemove](size_t idx) {
		return std::find(toRemove.begin(), toRemove.end(), outInput[idx]) != toRemove.end();
	};
	size_t startIdx = 0;
	for (; startIdx < outInput.size() && isIgnored(startIdx); ++startIdx)
		;
	size_t endIdx = outInput.size();
	for (; endIdx > startIdx && isIgnored(endIdx - 1); --endIdx)
		;
	outInput = outInput.substr(startIdx, endIdx - startIdx);
	return;
}

void removeWhitespace(std::string& outInput)
{
	substituteChars(outInput, {std::pair<char, char>('\t', ' '), std::pair<char, char>('\n', ' '), std::pair<char, char>('\r', ' ')});
	removeChars(outInput, {' '});
	return;
}

std::vector<std::string> tokenise(std::string_view s, char delimiter, std::initializer_list<std::pair<char, char>> escape)
{
	auto end = s.cend();
	auto start = end;

	std::stack<std::pair<char, char>> escapeStack;
	std::vector<std::string> v;
	bool bEscaping = false;
	bool bSkipThis = false;
	for (auto it = s.cbegin(); it != end; ++it)
	{
		if (bSkipThis)
		{
			bSkipThis = false;
			continue;
		}
		bSkipThis = bEscaping && *it == '\\';
		if (bSkipThis)
		{
			continue;
		}
		if (*it != delimiter || bEscaping)
		{
			if (start == end)
			{
				start = it;
			}
			for (auto e : escape)
			{
				if (bEscaping && *it == e.second)
				{
					if (e.first == escapeStack.top().first)
					{
						escapeStack.pop();
						bEscaping = !escapeStack.empty();
						break;
					}
				}
				if (*it == e.first)
				{
					bEscaping = true;
					escapeStack.push(e);
					break;
				}
			}
			bSkipThis = false;
			continue;
		}
		if (start != end)
		{
			v.emplace_back(start, it);
			start = end;
		}
		bSkipThis = false;
	}
	if (start != end)
	{
		v.emplace_back(start, end);
	}
	return v;
}

void substituteChars(std::string& outInput, std::initializer_list<std::pair<char, char>> replacements)
{
	std::string::iterator iter = outInput.begin();
	while (iter != outInput.end())
	{
		for (auto const replacement : replacements)
		{
			if (*iter == replacement.first)
			{
				*iter = replacement.second;
				break;
			}
		}
		++iter;
	}
	return;
}

bool isCharEnclosedIn(std::string_view str, size_t idx, std::pair<char, char> wrapper)
{
	size_t idx_1 = idx - 1;
	size_t idx1 = idx + 1;
	return idx_1 < str.length() && idx1 < str.length() && str[idx_1] == wrapper.first && str[idx1] == wrapper.second;
}
} // namespace utils::strings
} // namespace le
