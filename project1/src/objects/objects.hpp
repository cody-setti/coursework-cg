#pragma once

// --- INCLUSIONS ---
#include "geometry.hpp"


// --- THE OBJECTS OF THE SCENE ---
Mesh build_table();
Mesh build_wine_glass();
Mesh build_cheese();


Mesh build_rat();
inline constexpr float kRatScale = 0.03f;             // 19 modelling units, shrunk to fit the table
inline constexpr float kRatBelly = 3.0f * kRatScale;  // half the body's height, at that scale


struct Mousetrap {
	Mesh body;    // the wooden base, the staple and the trigger
	Mesh hammer;  // the bar that swings
};
Mousetrap build_mousetrap();
inline constexpr float kTrapHinge = 0.06f;
