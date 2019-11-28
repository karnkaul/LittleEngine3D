#include <algorithm>
#include <fstream>
#include <stack>
#include "le3d/core/utils.hpp"

namespace le
{
std::string utils::readFile(std::string_view path)
{
	std::ifstream file(path.data());
	std::stringstream buf;
	if (file.good())
	{
		buf << file.rdbuf();
	}
	return buf.str();
}

std::vector<u8> utils::readBytes(std::string_view path)
{
	std::string sPath(path);
	std::ifstream file(path.data(), std::ios::binary);
	std::vector<u8> buf;
	if (file.good())
	{
		buf = std::vector<u8>(std::istreambuf_iterator<char>(file), {});
	}
	return buf;
}

namespace utils::strings
{
void toLower(std::string& outString)
{
	std::transform(outString.begin(), outString.end(), outString.begin(), ::tolower);
}

void toUpper(std::string& outString)
{
	std::transform(outString.begin(), outString.end(), outString.begin(), ::toupper);
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
		catch (const std::invalid_argument&)
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
		catch (const std::exception&)
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
		catch (const std::exception&)
		{
			ret = defaultValue;
		}
	}
	return ret;
}

std::string toText(std::vector<u8> rawBuffer)
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
}

void removeWhitespace(std::string& outInput)
{
	substituteChars(outInput, {std::pair<char, char>('\t', ' '), std::pair<char, char>('\n', ' '), std::pair<char, char>('\r', ' ')});
	removeChars(outInput, {' '});
}

std::vector<std::string> tokenise(std::string_view s, char delimiter, std::initializer_list<std::pair<char, char>> escape)
{
	auto end = s.cend();
	auto start = end;

	std::stack<std::pair<char, char>> escapeStack;
	std::vector<std::string> v;
	bool escaping = false;
	for (auto it = s.cbegin(); it != end; ++it)
	{
		if (*it != delimiter || escaping)
		{
			if (start == end)
			{
				start = it;
			}
			for (auto e : escape)
			{
				if (escaping && *it == e.second)
				{
					if (e.first == escapeStack.top().first)
					{
						escapeStack.pop();
						escaping = !escapeStack.empty();
						break;
					}
				}
				if (*it == e.first)
				{
					escaping = true;
					escapeStack.push(e);
					break;
				}
			}
			continue;
		}
		if (start != end)
		{
			v.emplace_back(start, it);
			start = end;
		}
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
		for (const auto replacement : replacements)
		{
			if (*iter == replacement.first)
			{
				*iter = replacement.second;
				break;
			}
		}
		++iter;
	}
}

bool isCharEnclosedIn(std::string_view str, size_t idx, std::pair<char, char> wrapper)
{
	size_t idx_1 = idx - 1;
	size_t idx1 = idx + 1;
	return idx_1 < str.length() && idx1 < str.length() && str[idx_1] == wrapper.first && str[idx1] == wrapper.second;
}
} // namespace utils::strings
} // namespace le
