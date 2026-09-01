#pragma once

// --- INCLUSIONS ---
#include "geometry.hpp"


// --- THE OBJECTS OF THE SCENE ---
// Five objects, one per file in this directory, each one a composition of simple
// primitives: no model file is loaded and no library builds them for us. The table,
// the cheese and the wine glass are put together out of the primitives in
// geometry.hpp; the rat and the mousetrap were modelled the hard way, vertex by
// vertex, and their files are mostly the resulting tables of numbers.
//
// Each function returns its object's geometry READY TO PLACE: in the same units as
// every other object here, with y = 0 at the surface it stands on, and with its origin
// already wherever that object naturally turns about. Where it ends up on the table,
// and how it moves once it is there, is decided in scene.cpp -- geometry and animation
// are kept apart on purpose, so that neither has to know how the other works.
//
// Where an object cannot quite keep that promise on its own -- the rat has to be stood
// on its paws, the trap's hammer has to be told where its hinge is -- the missing number
// is published HERE, beside the function that owes it, rather than being copied into
// scene.cpp and kept in step by hand.
Mesh build_table();
Mesh build_cheese();
Mesh build_wine_glass();


// --- THE RAT ---
// It was modelled by hand at a comfortable size -- facing along -X, nose at x = 0, the
// tip of its tail out at x = 19, paws resting on y = 0 -- and build_rat() shrinks it to
// the table's scale and re-centres it on its own middle before handing it over. Centring
// it is what lets it somersault about its middle rather than about its nose.
//
// Which leaves the scene exactly one number: with the origin now at the rat's middle,
// this is how far that middle must be lifted for the paws to reach the tabletop.
Mesh build_rat();
inline constexpr float kRatScale = 0.03f;             // 19 modelling units, shrunk to fit the table
inline constexpr float kRatBelly = 3.0f * kRatScale;  // half the body's height, at that scale


// --- THE MOUSETRAP ---
// It comes back in two halves. Its wooden base and the metal staple bolted to it never
// move relative to each other, but the hammer -- the bar that snaps shut -- swings on
// its own hinge, so it needs a transformation matrix of its own and therefore a mesh of
// its own.
struct Mousetrap {
	Mesh body;    // the wooden base, the staple and the trigger
	Mesh hammer;  // the bar that swings
};
Mousetrap build_mousetrap();

// The height of the pin the hammer swings on. It is a fact about the vertices in
// mousetrap.cpp, so it is stated next to them and read from here by scene.cpp.
inline constexpr float kTrapHinge = 0.06f;
