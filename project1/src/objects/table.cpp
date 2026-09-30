// --- INCLUSIONS ---
#include "objects/objects.hpp"
#include "objects/palette.hpp"

#include "primary_transforms.hpp"  // The table is assembled out of boxes carried into place.

#include <glm/glm.hpp>             // For `vec2` and glm::length.
#include <cmath>                   // For std trigonometric functions.


namespace {
	// --- THE TABLE'S DIMENSIONS ---
	//
	// FRONT ELEVATION -- the XY plane, looking down the Z axis.
	//
	//        (-kHalfWidth, 0)                   (+kHalfWidth, 0)
	//                v                                         v
	//              ^ +=========================================+  ^
	//              | |               THE TOP SLAB              |  |   kThickness
	//              | +=========================================+  v
	//              |  \                                       /
	//              |      \                               /
	//              |          \                       /
	//              |              \               /
	//      kFloor  |                  \       /
	//              |                      X
	//              |                  /       \
	//              |              /               \
	//              |          /                       \
	//              |      /                               \            +------+   ^
	//              |  /                                       \        |      | kTimber
	//              v -------------------------------------------       +------+   v 
	//                 ^                                       ^
	//        (-kSpread, kFloor)               (+kSpread, kFloor)
	//
	//                 |<---- kSpread ---->|<--- kHalfWidth --->|
	//
	//
	// PLAN VIEW -- the XZ plane, looking straight down through the slab.
	//
	//   (-kHalfWidth, +kHalfDepth)    (+kHalfWidth, +kHalfDepth)
	//                v                                         v
	//              ^ +-----------------------------------------+
	//              | |                                         |
	//  kHalfDepth  | |=========================================|  ^
	//              | |                    H                    |  |   kFrameInset
	//              v |                    H                    |  v
	//                |                    H                    |  eInset)
	//                |=========================================|  ame mirrors
	//                |                                         |  set) to
	//                +-----------------------------------------+  
	//                ^                                         ^
	//   (-kHalfWidth, -kHalfDepth)    (+kHalfWidth, -kHalfDepth)
	//                 |<---- kSpread ---->|<--- kHalfWidth --->|  the same x measurements as in the front view
	//
	constexpr float kHalfWidth =  1.00f;
	constexpr float kHalfDepth =  0.80f;
	constexpr float kThickness =  0.06f;
	constexpr float kFloor     = -0.56f;
	constexpr float kFrameInset=  0.34f;
	constexpr float kTimber    =  0.05f;
	constexpr float kSpread    =  0.95f;
	
	void timber(Mesh& into, glm::vec2 a, glm::vec2 b, float z) {
		const glm::vec2 span   = b - a;
		const glm::vec2 middle = (a + b) * 0.5f;
	
		append(into, box(kTableLeg, kTableLegSide, kTableLegSide),
			  translation(middle.x, middle.y, z)
			* rotation_z(std::atan2(span.y, span.x))
			* scaling(glm::length(span), kTimber, kTimber)
		);
	}
	
}


Mesh build_table() {
	Mesh table;

	// THE TOP. One slab, hung below y = 0 so that its upper face lands exactly on it.
	append(table, box(kTableTop, kTableSide, kTableBase),
		  translation(0.0f, -kThickness * 0.5f, 0.0f)
		* scaling(kHalfWidth * 2.0f, kThickness, kHalfDepth * 2.0f)
	);

	// THE LEGS. Two X-shaped frames, one towards each end of the table, each made of
	// a pair of timbers crossing from the floor up to the underside of the slab.
	for (const float z : { -kFrameInset, kFrameInset }) {
		timber(table, { -kSpread, kFloor }, {  kSpread, -kThickness }, z);
		timber(table, {  kSpread, kFloor }, { -kSpread, -kThickness }, z);
	}

	// THE STRETCHER. One timber running the depth of the table where the two frames
	// cross, which is what keeps a real trestle table from folding up sideways.
	append(table, box(kTableLeg, kTableLegSide, kTableLegSide),
		  translation(0.0f, (kFloor - kThickness) * 0.5f, 0.0f)
		* scaling(kTimber, kTimber, kFrameInset * 2.0f)
	);

	return table;
}
