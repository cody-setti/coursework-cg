#pragma once

// --- INCLUSIONS ---
#include <glm/glm.hpp>


// --- THE PALETTE ---
// Every colour in the scene, gathered here so that requirement 1 -- five or more
// objects of DIFFERENT colours -- can be checked at a glance instead of by hunting
// through five files.
//
// Where an object lists more than one shade, they are the top, the sides and the
// underside of one painted surface, picked by hand. See the note in geometry.hpp:
// with no lighting allowed, that difference is the only thing stopping a solid from
// arriving on screen as a flat silhouette. Nothing here is computed from a light.

// --- THE TABLE: red-painted pine ---
inline const glm::vec3 kTableTop     { 1.00f, 0.00f, 0.00f };  // the surface everything stands on
inline const glm::vec3 kTableSide    { 0.72f, 0.02f, 0.02f };  // the edge of the slab
inline const glm::vec3 kTableBase    { 0.52f, 0.03f, 0.03f };  // its underside, in shadow
inline const glm::vec3 kTableLeg     { 0.80f, 0.02f, 0.02f };  // the crossed timbers below
inline const glm::vec3 kTableLegSide { 0.60f, 0.03f, 0.03f };

// --- THE RAT: brown, with bare skin ---
inline const glm::vec3 kFur          { 0.60f, 0.30f, 0.00f };
inline const glm::vec3 kSkin         { 1.00f, 0.90f, 0.80f };  // face, inner ears, paws, tail

// --- THE MOUSETRAP: a pine base with steel hardware ---
inline const glm::vec3 kWood         { 0.90f, 0.60f, 0.00f };
inline const glm::vec3 kSteel        { 0.50f, 0.50f, 0.50f };

// --- THE CHEESE: the bait ---
inline const glm::vec3 kCheeseCut    { 1.00f, 0.92f, 0.45f };  // the pale flesh where it was cut
inline const glm::vec3 kCheeseTop    { 0.93f, 0.76f, 0.20f };  // the top of the wheel
inline const glm::vec3 kCheeseRind   { 0.80f, 0.62f, 0.12f };  // the deeper yellow of the rind
inline const glm::vec3 kCheeseHole   { 0.55f, 0.40f, 0.05f };  // and the holes in the cut faces

// --- THE WINE GLASS ---
inline const glm::vec3 kGlass        { 0.86f, 0.89f, 0.93f };  // pale, barely-there glass
inline const glm::vec3 kWine         { 0.35f, 0.03f, 0.16f };  // and what is left in the bottom
