// --- INCLUSIONS ---
#include "objects/objects.hpp"
#include "objects/palette.hpp"

#include <vector>


namespace {
	// --- THE SILHOUETTE OF THE GLASS ---
	// |               7     <- [7]  r=0.082  h=0.350  kGlass   the rim
	// |               |
	// |               ||
	// |                |
	// |                |
	// |                |
	// |                |
	// |               /6    <- [6]  r=0.084  h=0.262  kGlass   empty glass again
	// |::::::::::::::5/     <- [5]  r=0.076  h=0.245  kWine    surface of the wine
	// |::::::::::::///
	// |::::::::::://
	// |::::::::///
	// |::// 4               <- [4]  r=0.028  h=0.175  kWine    bowl opens, wine begins
	// |  3/                 <- [3]  r=0.016  h=0.155  kGlass   top of the stem
	// |  |
	// |  |
	// |  |          the stem: [2]->[3] is near enough parallel-sided
	// |  |
	// |  ||
	// |   |
	// |   2                 <- [2]  r=0.018  h=0.055  kGlass   foot of the stem
	// |   \\
	// |      \
	// |        \\\\\\\
	// 0________________1    <- [1]  r=0.085  h=0.000  kGlass   rim of the foot
	// ^
	// [0] r=0.000  h=0.000 — sits on the axis, so this ring collapses to a point
	
	const Profile kProfile = {
		{ 0.000f, 0.000f, kGlass },  // [0]
		{ 0.085f, 0.000f, kGlass },  // [1]
		{ 0.018f, 0.055f, kGlass },  // [2]
		{ 0.016f, 0.155f, kGlass },  // [3]
		//
		{ 0.028f, 0.175f, kWine  },  // [4]
		{ 0.076f, 0.245f, kWine  },  // [5]
		//
		{ 0.084f, 0.262f, kGlass },  // [6]
		{ 0.082f, 0.350f, kGlass },  // [7]
	};
	
	constexpr int kSegments = 8;
	
}  // namespace


Mesh build_wine_glass() {
	return revolve(kProfile, kSegments);
}
