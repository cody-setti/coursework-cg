// --- INCLUSIONS ---
#include "objects/objects.hpp"
#include "objects/palette.hpp"

#include <vector>


namespace {

// --- THE SILHOUETTE OF THE GLASS ---
// Read from the bottom up, this is the outline you would see if you cut the glass in
// half: how far each point stands out from the central axis, and how high up it is.
// Spinning it right the way round makes the solid, so the three parts anyone would
// name -- a flat foot, a thin stem, a bowl that flares and closes again -- are here
// as three runs of numbers rather than as three separate meshes.
//
// The colour of each point is the colour the surface takes there, and it blends
// smoothly into the next one. That is the whole trick behind the wine: the lower part
// of the bowl is simply painted red, and the short gap to the next point turns into
// the surface of what is left in the glass.
const std::vector<ProfilePoint> kProfile = {
	{ 0.000f, 0.000f, kGlass },  // the centre of the foot
	{ 0.085f, 0.000f, kGlass },  // out to its rim
	{ 0.018f, 0.055f, kGlass },  // in and up to the foot of the stem
	{ 0.016f, 0.155f, kGlass },  // the stem, near enough parallel-sided. Drawing the foot
	                             // and the stem as one straight taper saves a point, but
	                             // the two cones then meet at nothing and the whole thing
	                             // reads as an hourglass: the stem is what makes it a glass
	{ 0.028f, 0.175f, kWine  },  // the bowl opens out, and the wine begins
	{ 0.076f, 0.245f, kWine  },  // up to the surface of the wine
	{ 0.084f, 0.262f, kGlass },  // above it, empty glass again
	{ 0.082f, 0.350f, kGlass },  // and in a little to the rim
};

// How many flat facets stand in for the circle. Six: the glass is meant to look cut
// rather than blown, and it keeps company with a twelve-sided cheese and a rat made of
// flat planes. This is the one number to raise if it ever looks too crude -- nothing
// else in the file has to change with it.
//
// Eight points of silhouette at six facets comes to 234 vertices, which makes the glass
// the second simplest object on the table, after the mousetrap. It started at 936.
constexpr int kSegments = 6;

}  // namespace


// Left open at the top, on purpose: with nothing capping the rim you can see down
// into the bowl, which is what makes it read as a glass rather than as a skittle.
Mesh build_wine_glass() {
	return revolve(kProfile, kSegments);
}
