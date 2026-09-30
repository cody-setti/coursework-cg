// --- INCLUSIONS ---
#include "objects/objects.hpp"
#include "objects/palette.hpp"

#include "primary_transforms.hpp"

#include <vector>


namespace {
	
	// --- THE WHEEL ---
	constexpr float kRadius = 0.13f;
	constexpr float kHeight = 0.14f;
	constexpr float kSweep  = 5.24f;
	constexpr int   kFacets = 10   ;
	
	
	//                3_________________2   <- [3] r=0.000  h=0.140  kCheeseTop   centre of the lid
	//       ^        |                 |      [2] r=0.130  h=0.140  kCheeseTop   top of the rind
	//       |        |                 |
	//       |        |                 |
	//     0.140      |    the rind     |
	//       |        |                 |
	//       |        |                 |
	//       v        0_________________1   <- [0] r=0.000  h=0.000  kCheeseRind  centre of the base
	//                |<---- 0.130 ---->|      [1] r=0.130  h=0.000  kCheeseRind  rim of the base
	//                ^
	//                the Y axis. The rings at [0] and [3] collapse to a single point.
	const Profile kProfile = {
		{ 0.0f    , 0.0f    , kCheeseRind },  // [0] the centre of the base
		{ kRadius , 0.0f    , kCheeseRind },  // [1] out to the rim
		{ kRadius , kHeight , kCheeseTop  },  // [2] up the rind
		{ 0.0f    , kHeight , kCheeseTop  },  // [3] and back in across the top
	};
	
	// --- THE HOLES ---
	// Nothing is actually bored through the mesh.
	// a dark disc laid on the cut face reads as a hole perfectly well.
	constexpr int kHoleFacets = 8;
	
	enum class CutFace {
		Near,  // the face at the start of the sweep, looking back along -Z
		Far    // and the one at its end, looking forward along +Z
	};
	
	void hole(Mesh& into, CutFace face, float x, float y, float radius) {
		const float facing = (face == CutFace::Near) ? -1.0f :  1.0f;
		const float angle  = (face == CutFace::Near) ?  0.0f : -kSweep;
	
		append(into, revolve({ { 0.0f, 0.0f, kCheeseHole }, { 1.0f, 0.0f, kCheeseHole } }, kHoleFacets),
			  rotation_y(angle)
			* translation(x, y, facing * 0.001f)
			
			* rotation_x(facing * kTau * 0.25f)
			* scaling(radius, 1.0f, radius)
		);
	}
	
}



Mesh build_cheese() {
	// The wheel, with the slice-shaped notch left open and its two cut faces filled
	// in with the pale colour of freshly cut cheese.
	Mesh cheese = revolve(kProfile, kFacets, kSweep, kCheeseCut);
	
	
//           NEAR face                       FAR face
//   lid +-----------------------------+ +-----------------------------+
//       |                             | |                             |
//       |                             | |                             |
//       |                             | |                             |
//       |                             | |                             |
//       |                             | |                             |
//       |         @@@@@               | |                             |
//       |       @@@@ @@@@             | |             @@@@@           |
//       |       @@@ A @@@             | |           @@@@ @@@          |
//       |       @@@@ @@@@             | |           @@@ D @@          |
//       |         @@@@@    @ @@       | |            @@@ @@@          |
//       |                @@ B @@      | |                    @@ @     |
//       |        @@@@@   @@@ @@@      | |                   @@ E @    |
//       |        @ C @     @@@@       | |                    @@ @     |
//       |                             | |                             |
//       |                             | |                             |
//       |                             | |                             |
// table +-----------------------------+ +-----------------------------+
//       ^                             ^
//     x = 0                     x = kRadius
//    (the axis)                  (the rind)
//
//   A  x=0.050  y=0.075  r=0.022      D  x=0.068  y=0.068  r=0.020
//   B  x=0.090  y=0.042  r=0.017      E  x=0.100  y=0.038  r=0.014
//   C  x=0.045  y=0.032  r=0.013
	hole(cheese, CutFace::Near, 0.050f, 0.075f, 0.022f);  // A
	hole(cheese, CutFace::Near, 0.090f, 0.042f, 0.017f);  // B
	hole(cheese, CutFace::Near, 0.045f, 0.032f, 0.013f);  // C

	hole(cheese, CutFace::Far , 0.068f, 0.068f, 0.020f);  // D
	hole(cheese, CutFace::Far , 0.100f, 0.038f, 0.014f);  // E

	return cheese;
}
