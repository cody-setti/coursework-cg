// --- SCENE ---
// WHAT is in the world and WHERE it stands.

#pragma once                             // `#include` literally pastes a file's text in place.
                                         // `#pragma once` protects `scene.hpp` from being double pasted.

#include <vector>                        // `std::vector` is a resizable array.

#include <glm/glm.hpp>                   // GL Mathematics: provides GLSL-compatible vector and matrix types.

#include "world/transform.hpp"           // `xform::Transform` and `xform::modelMatrix()`.


namespace scn {

// TYPES
// One name per .obj file.
enum ModelID {
	// External models.
	MODEL_GRASS ,
	MODEL_PINE  ,
	MODEL_OAK   ,
	MODEL_SHIELD,
	MODEL_MOON  ,
	MODEL_NAVI  ,
	// Transitional model.
	MODEL_HUT   ,
	// Internal models.
	MODEL_MAN   ,
	MODEL_CANDLE,
	MODEL_POT   ,
	MODEL_HEART ,
	
	// Since each value of an `enum` is just an integer (starting from 0),
	// if we add an extra value at the end, it will hold the number of models
	// being used. Very smart!
	MODEL_COUNT
};

struct Scene {
	std::vector<glm::mat4> statics[scn::MODEL_COUNT];  // e.g.:                                                                      
	                                                   //       statics[MODEL_GRASS]  →  [ M ]                   1 copy              
	                                                   //       statics[MODEL_PINE ]  →  [ M, M, M, M, ..., M ]  9 copies            
	                                                   //       statics[MODEL_OAK  ]  →  [ M, M, M ]             3 copies            
	                                                   //       statics[MODEL_NAVI ]  →  [ ]                     empty (it's dynamic)
	                                                   //                            ...                                             
	
	xform::Transform       navi, heart;                // Their M is rebuilt every frame.
};


// CONSTANTS
constexpr float TAU = 6.28318530718f;  // τ: one full turn, in radians.

// The model order of `OBJ_PATHS[]` MUST mirror exactly that of `scn::ModelID`.
const char* const OBJ_PATHS[scn::MODEL_COUNT] = {
	"assets/grass/grass.obj"  ,
	"assets/pine/pine.obj"    ,
	"assets/oak/oak.obj"      ,
	"assets/shield/shield.obj",
	"assets/moon/moon.obj"    ,
	"assets/navi/navi.obj"    ,
	"assets/hut/hut.obj"      ,
	"assets/man/man.obj"      ,
	"assets/candle/candle.obj",
	"assets/pot/pot.obj"      ,
	"assets/heart/heart.obj"  ,
};

// Sky box faces, in OpenGL's cube-map conventional order: +X, -X, +Y, -Y, +Z, -Z.
const char* const SKYBOX_FACE_PATHS[6] = {
	"assets/skybox/right.png" ,
	"assets/skybox/left.png"  ,
	"assets/skybox/top.png"   ,
	"assets/skybox/bottom.png",
	"assets/skybox/front.png" ,
	"assets/skybox/back.png"  ,
};

constexpr glm::vec3 EYE_START  = glm::vec3(  0.0f,  1.7f,   8.0f);
constexpr glm::vec3 TRGT_START = glm::vec3(  0.0f,  1.0f,   0.0f);

constexpr glm::vec3 EYE_MIN    = glm::vec3(-20.0f,  0.3f, -20.0f);
constexpr glm::vec3 EYE_MAX    = glm::vec3( 20.0f, 30.0f,  20.0f);

constexpr glm::vec3 AMBIENT_LIGHT    = glm::vec3(0.2f);
constexpr glm::vec3 NAVI_LIGHT_COLOR = glm::vec3(0.7f, 0.8f, 1.0f);  // Blue


// FUNCTIONS
// Adds one static copy of `model` to the scene.
// Its M is computed here, once, and never again.
inline void place(
	scn::Scene&  scene   ,
	scn::ModelID model   ,
	glm::vec3    position,
	glm::vec3    angles  ,
	glm::vec3    scale
) {
	scene.statics[model].push_back( xform::modelMatrix({position, angles, scale}) );
}

inline void arrangeScene(scn::Scene& scene) {
	// --- STATIC ---
	//               model           , position                 , angles (radians)                        , scale
	
	// EXTERNAL
	scn::place(scene, scn::MODEL_GRASS , {  0.0f,  0.0f ,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.5f  ));
	
	scn::place(scene, scn::MODEL_PINE  , { -9.0f,  0.0f , -12.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.6f  ));
	scn::place(scene, scn::MODEL_PINE  , { -5.0f,  0.0f ,  -9.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.5f  ));
	scn::place(scene, scn::MODEL_PINE  , {  5.0f,  0.0f ,  -7.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(3.0f  ));
	scn::place(scene, scn::MODEL_PINE  , {  5.0f,  0.0f ,   7.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.1f  ));
	scn::place(scene, scn::MODEL_PINE  , {  0.0f,  0.0f ,  14.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.2f  ));
	scn::place(scene, scn::MODEL_PINE  , { 10.0f,  0.0f ,   9.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.5f  ));
	scn::place(scene, scn::MODEL_PINE  , { -7.0f,  0.0f ,  13.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.7f  ));
	scn::place(scene, scn::MODEL_PINE  , {-10.0f,  0.0f ,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(2.0f  ));
	scn::place(scene, scn::MODEL_PINE  , {-11.0f,  0.0f ,   4.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.0f  ));
	
	scn::place(scene, scn::MODEL_OAK   , {  6.0f,  0.0f ,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.2f  ));
	scn::place(scene, scn::MODEL_OAK   , {  0.0f,  0.0f ,  -5.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.9f  ));
	scn::place(scene, scn::MODEL_OAK   , { -5.0f,  0.0f ,   6.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.6f  ));
	
	scn::place(scene, scn::MODEL_MOON  , {-20.0f, 70.0f ,   0.0f}, { 0.2f *TAU,  0.25f*TAU, 0.0f*TAU}, glm::vec3(0.015f));
	
	scn::place(scene, scn::MODEL_SHIELD, {-2.46f,  0.01f,   0.6f}, {-0.04f*TAU, -0.25f*TAU, 0.0f*TAU}, glm::vec3(0.2f  ));
	
	// TRANSITIONAL
	// The hut's own floor is modeled at y = 0 too: lifted a hair, so it doesn't flicker against the grass.
	scn::place(scene, scn::MODEL_HUT   , {  0.0f,  0.05f,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.0f  ));
	
	// INTERNAL
	scn::place(scene, scn::MODEL_MAN   , {  0.0f,  0.08f,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.01f ));
	scn::place(scene, scn::MODEL_CANDLE, { -1.5f,  0.08f,  -0.8f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.07f ));
	scn::place(scene, scn::MODEL_POT   , {  1.8f,  0.08f,  -0.8f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.03f ));
	scn::place(scene, scn::MODEL_POT   , {  1.8f,  0.08f,   0.6f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.03f ));
	
	
	// --- DYNAMIC ---
	// Only their starting values.
	//            position                , angles (radians)                              , scale
	scene.navi  = { { -2.0f,  1.5f,  4.0f}, glm::vec3(0.0f*TAU,  0.0f*TAU, 0.0f*TAU), glm::vec3(0.05f ) };
	scene.heart = { {  1.8f, 0.35f, -0.1f}, glm::vec3(0.0f*TAU, 0.25f*TAU, 0.0f*TAU), glm::vec3(0.03f ) };
}

}  // namespace scn
