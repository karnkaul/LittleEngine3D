#pragma once
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/ubyte.hpp"

namespace le
{
// \brief Compressed wrapper struct for SFML Color
struct Colour
{
public:
	static const Colour Black;
	static const Colour White;
	static const Colour Red;
	static const Colour Green;
	static const Colour Blue;
	static const Colour Yellow;
	static const Colour Magenta;
	static const Colour Cyan;
	static const Colour Transparent;

	static Colour lerp(Colour min, Colour max, f32 alpha);

	UByte r;
	UByte g;
	UByte b;
	UByte a;

	// Warning: All inputs must be unsigned ints!
	Colour(UByte r = 255, UByte g = 255, UByte b = 255, UByte a = 255);

	Colour& operator+=(Colour rhs);
	Colour& operator-=(Colour rhs);

	std::string toString() const;
};

Colour operator+(Colour lhs, Colour rhs);
Colour operator-(Colour lhs, Colour rhs);
Colour& operator*=(f32 n, Colour& colour);
Colour operator*(f32 n, Colour colour);

bool operator==(Colour lhs, Colour rhs);
bool operator!=(Colour lhs, Colour rhs);
} // namespace le
