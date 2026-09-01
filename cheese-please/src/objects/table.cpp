// --- INCLUSIONS ---
#include "objects/objects.hpp"
#include "objects/palette.hpp"

#include "primary_transforms.hpp"  // The table is assembled out of boxes carried into place.

#include <glm/glm.hpp>             // For `vec2` and glm::length.
#include <cmath>                   // For std trigonometric functions.


namespace {

// --- THE TABLE'S DIMENSIONS ---
// The top surface is the plane y = 0. That is the single most useful fact about this
// object: everything else in the scene is placed at y = 0 and is therefore standing
// on the table without any further arithmetic. The rest of the table hangs below.
constexpr float kHalfWidth =  1.00f;  // out to the left and right edges. A long refectory
                                      // table: half as wide again as it once was, which is
                                      // what gives the five objects room to stand apart
constexpr float kHalfDepth =  0.80f;  // out to the near and far edges
constexpr float kThickness =  0.06f;  // how thick the top slab is
constexpr float kFloor     = -0.56f;  // where the feet of the legs reach
constexpr float kFrameInset=  0.34f;  // how far in from the edges the leg frames stand
constexpr float kTimber    =  0.05f;  // how stout those timbers are
constexpr float kSpread    =  0.95f;  // how far apart the feet of one X stand

// One timber running from `a` to `b` in the XY plane, standing at depth `z`.
//
// This is the whole point of building an object out of primitives: `box` makes a
// unit cube at the origin, and three primary transformations turn it into a plank.
// Read right to left -- stretch the cube into a plank as long as the gap between the
// two ends, tip it over to the angle that gap lies at, and carry it to the middle of
// the gap.
void timber(Mesh& into, glm::vec2 a, glm::vec2 b, float z) {
	const glm::vec2 span   = b - a;
	const glm::vec2 middle = (a + b) * 0.5f;

	append(into, box(kTableLeg, kTableLegSide, kTableLegSide),
		  translation(middle.x, middle.y, z)
		* rotation_z(std::atan2(span.y, span.x))
		* scaling(glm::length(span), kTimber, kTimber)
	);
}

}  // namespace


// A trestle table: one slab resting on two crossed frames, tied together by a
// stretcher along the bottom. Six boxes in all -- which is exactly why requirement 2
// is satisfied by it and would not be by a single box.
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
