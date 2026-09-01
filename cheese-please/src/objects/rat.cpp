// --- INCLUSIONS ---
#include "objects/objects.hpp"
#include "objects/palette.hpp"

#include "primary_transforms.hpp"  // The two corrections build_rat applies before handing the mesh over.

#include <vector>


namespace {

// --- THE RAT ---
// Modelled by hand, vertex by vertex, in its own coordinates: facing along -X with
// its nose at x = 0 and the tip of its tail out at x = 19, and with its paws resting
// on y = 0. Working at that comfortable size is far easier than typing out
// three-decimal table coordinates by hand -- so the two corrections that bring the rat
// into the scene's units are applied at the foot of THIS file, where the modelling
// units are known, rather than being left for scene.cpp to remember on its behalf.

// The middle of the body, in the modelling units above. build_rat brings this point
// onto the origin, which is what makes the rat turn and somersault about its middle
// rather than about its nose.
constexpr float kMiddleX = 9.5f;  // half the 19 units from nose to tail tip
constexpr float kMiddleY = 3.0f;  // half the 6 units from paws to shoulder
constexpr float kMiddleZ = 3.0f;  // half the 6 units across the body

// kRatBelly is the number scene.cpp stands the rat on the table with, so it has to stay
// equal to the height this file just moved away. Checking it here means the two cannot
// drift apart quietly: change kMiddleY alone and the build stops, rather than the rat
// sinking into the tabletop.
static_assert(kRatBelly == kMiddleY * kRatScale,
              "kRatBelly in objects.hpp must match the rat's modelled middle");
//
// The split into two tables is the rat's two-tone colouring, and it falls exactly
// where the original code passed a hardcoded "192" to glDrawArrays. Written as two
// named tables, the boundary is the array's own length: the compiler counts it, and
// it can no longer drift out of step with the vertices when one of them is edited.
const std::vector<float> kRatFur = {
		/*Body*/
	//belly
	6.0, 2.0, 0.0,
	12.0, 2.0, 0.0,
	12.0, 2.0, 6.0,

	6.0, 2.0, 0.0,
	12.0, 2.0, 6.0,
	6.0, 2.0, 6.0,

	//left side
	6.0, 2.0, 0.0,
	6.0, 3.0, 0.0,
	12.0, 3.0, 0.0,

	6.0, 2.0, 0.0,
	12.0, 3.0, 0.0,
	12.0, 2.0, 0.0,

	6.0, 3.0, 0.0,
	6.0, 5.0, 2.0,
	12.0, 5.0, 2.0,

	6.0, 3.0, 0.0,
	12.0, 5.0, 2.0,
	12.0, 3.0, 0.0,

	//right side
	6.0, 2.0, 6.0,
	6.0, 3.0, 6.0,
	12.0, 3.0, 6.0,

	6.0, 2.0, 6.0,
	12.0, 3.0, 6.0,
	12.0, 2.0, 6.0,

	6.0, 3.0, 6.0,
	6.0, 5.0, 4.0,
	12.0, 5.0, 4.0,

	6.0, 3.0, 6.0,
	12.0, 5.0, 4.0,
	12.0, 3.0, 6.0,

	//top
	6.0, 5.0, 2.0,
	12.0, 5.0, 2.0,
	12.0, 5.0, 4.0,

	6.0, 5.0, 2.0,
	12.0, 5.0, 4.0,
	6.0, 5.0, 4.0,

	//booty
	12.0, 2.0, 0.0,
	12.0, 2.0, 6.0,
	14.0, 2.0, 4.0,

	12.0, 2.0, 0.0,
	14.0, 2.0, 4.0,
	14.0, 2.0, 2.0,

	12.0, 2.0, 0.0,
	14.0, 2.0, 2.0,
	14.0, 3.0, 2.0,

	12.0, 2.0, 0.0,
	14.0, 3.0, 2.0,
	12.0, 3.0, 0.0,

	12.0, 2.0, 6.0,
	14.0, 2.0, 4.0,
	14.0, 3.0, 4.0,

	12.0, 2.0, 6.0,
	14.0, 3.0, 4.0,
	12.0, 3.0, 6.0,

	14.0, 2.0, 2.0,
	14.0, 2.0, 4.0,
	14.0, 3.0, 4.0,

	14.0, 2.0, 2.0,
	14.0, 3.0, 4.0,
	14.0, 3.0, 2.0,

	12.0, 3.0, 0.0,
	12.0, 5.0, 2.0,
	14.0, 3.0, 2.0,

	12.0, 3.0, 6.0,
	12.0, 5.0, 4.0,
	14.0, 3.0, 4.0,

	14.0, 3.0, 2.0,
	14.0, 3.0, 4.0,
	12.0, 5.0, 2.0,

	14.0, 3.0, 4.0,
	12.0, 5.0, 2.0,
	12.0, 5.0, 4.0,

		/* neck */
	6.0, 2.0, 0.0,
	6.0, 3.0, 0.0,
	4.0, 3.0, 1.0,

	6.0, 2.0, 6.0,
	6.0, 3.0, 6.0,
	4.0, 3.0, 5.0,

	6.0, 2.0, 0.0,
	4.0, 3.0, 1.0,
	6.0, 2.0, 6.0,

	4.0, 3.0, 1.0,
	6.0, 2.0, 6.0,
	4.0, 3.0, 5.0,

	6.0, 3.0, 0.0,
	6.0, 5.0, 2.0,
	4.0, 3.0, 1.0,

	6.0, 3.0, 6.0,
	6.0, 5.0, 4.0,
	4.0, 3.0, 5.0,

	6.0, 5.0, 2.0,
	6.0, 5.0, 4.0,
	4.0, 5.0, 4.0,

	6.0, 5.0, 2.0,
	4.0, 5.0, 4.0,
	4.0, 5.0, 2.0,

	6.0, 5.0, 2.0,
	4.0, 5.0, 2.0,
	4.0, 3.0, 1.0,

	6.0, 5.0, 4.0,
	4.0, 5.0, 4.0,
	4.0, 3.0, 5.0,

		/* legs */
	//left-front
	6.0, 2.0, 0.0,
	6.0, 0.0, 0.0,
	7.0, 0.0, 0.0,

	6.0, 2.0, 0.0,
	7.0, 0.0, 0.0,
	7.0, 2.0, 0.0,

	6.0, 2.0, 0.0,
	6.0, 2.0, 1.0,
	6.0, 0.0, 1.0,

	6.0, 2.0, 0.0,
	6.0, 0.0, 1.0,
	6.0, 0.0, 0.0,

	6.0, 2.0, 1.0,
	7.0, 2.0, 1.0,
	7.0, 0.0, 1.0,

	6.0, 2.0, 1.0,
	7.0, 0.0, 1.0,
	6.0, 0.0, 1.0,

	7.0, 2.0, 0.0,
	7.0, 2.0, 1.0,
	7.0, 0.0, 1.0,

	7.0, 2.0, 0.0,
	7.0, 0.0, 1.0,
	7.0, 0.0, 0.0,

	6.0, 0.0, 0.0,
	7.0, 0.0, 0.0,
	7.0, 0.0, 1.0,

	6.0, 0.0, 0.0,
	7.0, 0.0, 1.0,
	6.0, 0.0, 1.0,

	//right-front
	6.0, 2.0, 6.0,
	6.0, 0.0, 6.0,
	7.0, 0.0, 6.0,

	6.0, 2.0, 6.0,
	7.0, 0.0, 6.0,
	7.0, 2.0, 6.0,

	6.0, 2.0, 6.0,
	6.0, 2.0, 5.0,
	6.0, 0.0, 5.0,

	6.0, 2.0, 6.0,
	6.0, 0.0, 5.0,
	6.0, 0.0, 6.0,

	6.0, 2.0, 5.0,
	7.0, 2.0, 5.0,
	7.0, 0.0, 5.0,

	6.0, 2.0, 5.0,
	7.0, 0.0, 5.0,
	6.0, 0.0, 5.0,

	7.0, 2.0, 6.0,
	7.0, 2.0, 5.0,
	7.0, 0.0, 5.0,

	7.0, 2.0, 6.0,
	7.0, 0.0, 5.0,
	7.0, 0.0, 6.0,

	6.0, 0.0, 6.0,
	7.0, 0.0, 6.0,
	7.0, 0.0, 5.0,

	6.0, 0.0, 6.0,
	7.0, 0.0, 5.0,
	6.0, 0.0, 5.0,

	//left-back
	12.0, 2.0, 0.0,
	14.0, 2.0, 2.0,
	12.0, 0.0, 0.0,

	12.0, 2.0, 0.0,
	12.0, 2.0, 2.0,
	12.0, 0.0, 0.0,

	12.0, 2.0, 2.0,
	14.0, 2.0, 2.0,
	12.0, 0.0, 0.0,

	//right-back

	12.0, 2.0, 6.0,
	14.0, 2.0, 4.0,
	12.0, 0.0, 6.0,

	12.0, 2.0, 6.0,
	12.0, 2.0, 4.0,
	12.0, 0.0, 6.0,

	12.0, 2.0, 4.0,
	14.0, 2.0, 4.0,
	12.0, 0.0, 6.0,

		/* ears (back) */
	//left
	4.0, 6.0, 1.5,
	4.0, 5.0, 2.0,
	5.0, 4.0, 1.5,

	4.0, 6.0, 1.5,
	5.0, 4.0, 1.5,
	4.0, 3.0, 1.0,

	//right
	4.0, 6.0, 4.5,
	4.0, 5.0, 4.0,
	5.0, 4.0, 4.5,

	4.0, 6.0, 4.5,
	5.0, 4.0, 4.5,
	4.0, 3.0, 5.0,
};

// Face, inner ears, paws and tail: the parts with no fur on them.
const std::vector<float> kRatSkin = {
		/* face */
	4.0, 5.0, 2.0,
	4.0, 5.0, 4.0,
	0.0, 3.0, 3.0,

	4.0, 5.0, 2.0,
	4.0, 3.0, 1.0,
	0.0, 3.0, 3.0,

	4.0, 5.0, 4.0,
	4.0, 3.0, 5.0,
	0.0, 3.0, 3.0,

	4.0, 3.0, 5.0,
	4.0, 3.0, 1.0,
	0.0, 3.0, 3.0,

		/* ears (front) */
	4.0, 5.0, 2.0,
	4.0, 3.0, 1.0,
	4.0, 6.0, 1.5,

	4.0, 5.0, 4.0,
	4.0, 3.0, 5.0,
	4.0, 6.0, 4.5,

		/* paws */
	//left-front
	6.0, 0.0, 0.0,
	6.0, 0.0, 1.0,
	5.0, 0.0, 0.5,

	6.0, 0.0, 0.0,
	5.0, 0.0, 0.5,
	6.0, 0.5, 0.5,

	6.0, 0.0, 1.0,
	5.0, 0.0, 0.5,
	6.0, 0.5, 0.5,

	//right-front
	6.0, 0.0, 6.0,
	6.0, 0.0, 5.0,
	5.0, 0.0, 5.5,

	6.0, 0.0, 6.0,
	5.0, 0.0, 5.5,
	6.0, 0.5, 5.5,

	6.0, 0.0, 5.0,
	5.0, 0.0, 5.5,
	6.0, 0.5, 5.5,

	//left-back
	12.0, 0.0, 0.0,
	12.0, 0.0, 1.0,
	11.0, 0.0, 0.5,

	12.0, 0.0, 0.0,
	11.0, 0.0, 0.5,
	12.0, 0.5, 0.5,

	12.0, 0.0, 1.0,
	11.0, 0.0, 0.5,
	12.0, 0.5, 0.5,

	12.0, 0.0, 0.0,
	12.0, 0.0, 1.0,
	12.0, 0.5, 0.5,

	//right-back
	12.0, 0.0, 6.0,
	12.0, 0.0, 5.0,
	11.0, 0.0, 5.5,

	12.0, 0.0, 6.0,
	11.0, 0.0, 5.5,
	12.0, 0.5, 5.5,

	12.0, 0.0, 5.0,
	11.0, 0.0, 5.5,
	12.0, 0.5, 5.5,

	12.0, 0.0, 6.0,
	12.0, 0.0, 5.0,
	12.0, 0.5, 5.5,

		/* tail */
	14.0, 2.0, 2.5,
	14.0, 2.0, 3.5,
	19.0, 2.5, 3.0,

	14.0, 2.0, 2.5,
	14.0, 3.0, 2.5,
	19.0, 2.5, 3.0,

	14.0, 3.0, 2.5,
	14.0, 3.0, 3.5,
	19.0, 2.5, 3.0,

	14.0, 2.0, 3.5,
	14.0, 3.0, 3.5,
	19.0, 2.5, 3.0,
};

}  // namespace


Mesh build_rat() {
	Mesh modelled;
	append(modelled, kRatFur , kFur );
	append(modelled, kRatSkin, kSkin);

	// Shrink to the table's scale and bring the middle of the body onto the origin. Baked
	// into the vertices once, here, at start-up: it never changes while the program runs,
	// and doing it now leaves rat_transform in scene.cpp free to talk about nothing but
	// the animation.
	Mesh rat;
	append(rat, modelled,
		  scaling(kRatScale, kRatScale, kRatScale)
		* translation(-kMiddleX, -kMiddleY, -kMiddleZ)
	);
	return rat;
}
