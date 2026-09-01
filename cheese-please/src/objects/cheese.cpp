// --- INCLUSIONS ---
#include "objects/objects.hpp"
#include "objects/palette.hpp"

#include "transform.hpp"


namespace {

// --- THE WHEEL ---
constexpr float kRadius = 0.13f;   // how wide the little wheel of cheese is
constexpr float kHeight = 0.14f;   // and how tall. Deeper than a real cheese would be
                                   // cut, because we look down on the table at a slant:
                                   // a wheel three times wider than it is tall shows us
                                   // almost nothing but its lid.
constexpr float kSweep  = 5.24f;   // 300 degrees of it: the missing sixth is the slice
                                   // somebody has already cut and taken away
constexpr int   kFacets = 10;      // deliberately few. The whole scene is faceted, and a
                                   // cheese with a hundred sides would sit oddly beside a
                                   // rat made of thirty flat planes.

// Its silhouette, spun around the upright axis: out along the bottom, straight up the
// rind, and back in across the top. Four points -- that is the entire cheese.
//
// The colours are what make a plain cylinder read as a solid. The bottom is the deep
// waxy yellow of the rind all the way across; the top is lighter; and because the two
// ends of the rind band carry those two colours, the side shades smoothly from dark at
// the bottom to light at the top all by itself. No lighting involved, just two
// well-chosen numbers.
const std::vector<ProfilePoint> kProfile = {
	{ 0.0f    , 0.0f    , kCheeseRind },  // the centre of the base
	{ kRadius , 0.0f    , kCheeseRind },  // out to the rim
	{ kRadius , kHeight , kCheeseTop  },  // up the rind
	{ 0.0f    , kHeight , kCheeseTop  },  // and back in across the top
};

// --- THE HOLES ---
// The characteristic holes go on the two flat faces the missing slice exposed, which
// is exactly where you see them in a real cheese.
//
// Nothing is actually bored through the mesh: a dark disc laid on the cut face reads
// as a hole perfectly well, and costs six triangles rather than an algorithm for
// punching holes through a solid. `revolve` supplies the disc -- a silhouette running
// from the axis straight out to a radius of one, swung the whole way round, is a
// circle -- and six facets keeps it as chunky as everything around it.
constexpr int kHoleFacets = 6;

// Which of the two faces the slice left behind a hole belongs to. There are only ever
// these two, and each of them fixes BOTH the bearing to turn the disc to and the
// direction it has to face -- so neither is worth asking the caller for. Handing those
// out as two loose numbers is an invitation to pass a pair that disagree.
enum class CutFace {
	Near,  // the face at the start of the sweep, looking back along -Z
	Far    // and the one at its end, looking forward along +Z
};

// A cut face runs from the axis out to the rim (that is x, from 0 to kRadius) and from
// the table up to the top (y, from 0 to kHeight).
//
// NOTE THE MINUS SIGN ON THE FAR FACE. `revolve` lays its surface out at +phi, whereas
// rotation_y turns a bearing DOWNWARDS -- a bearing phi ends up at phi minus the
// argument. Turn a disc by +kSweep and it lands at -kSweep instead, which is a bearing
// well inside the wheel: the hole is then buried in the solid and simply never seen.
// The sign is written once, here, so that no call site can get it wrong again.
void hole(Mesh& into, CutFace face, float x, float y, float radius) {
	const float facing = (face == CutFace::Near) ? -1.0f :  1.0f;
	const float angle  = (face == CutFace::Near) ?  0.0f : -kSweep;

	append(into, revolve({ { 0.0f, 0.0f, kCheeseHole }, { 1.0f, 0.0f, kCheeseHole } }, kHoleFacets),
		  rotation_y(angle)                      // turn to the cut face this hole belongs to,
		* translation(x, y, facing * 0.001f)     // stand a hair proud of it so the two surfaces
		                                         // do not flicker against each other,
		* rotation_x(facing * kPi * 0.5f)        // tip the disc up to face the same way it does,
		* scaling(radius, 1.0f, radius)          // and size it. The disc lies in the XZ plane,
	);                                        // so Y is its thickness and goes untouched.
}

}  // namespace


Mesh build_cheese() {
	// The wheel, with the slice-shaped notch left open and its two cut faces filled
	// in with the pale colour of freshly cut cheese.
	Mesh cheese = revolve(kProfile, kFacets, kSweep, kCheeseCut);

	hole(cheese, CutFace::Near, 0.050f, 0.075f, 0.022f);
	hole(cheese, CutFace::Near, 0.090f, 0.042f, 0.017f);
	hole(cheese, CutFace::Near, 0.045f, 0.032f, 0.013f);

	hole(cheese, CutFace::Far , 0.068f, 0.068f, 0.020f);
	hole(cheese, CutFace::Far , 0.100f, 0.038f, 0.014f);

	return cheese;
}
