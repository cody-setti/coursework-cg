// --- SCENE ---
// WHAT is in the world and WHERE it stands.

#pragma once                             // `#include` literally pastes a file's text in place.
                                         // `#pragma once` protects `scene.hpp` from being double pasted.

#include <cstdint>                       // `std::uint32_t`: an unsigned integer of exactly 32 bits, like GLSL's `uint`.

#include <vector>                        // `std::vector` is a resizable array.

#include <glm/glm.hpp>                   // GL Mathematics: provides GLSL-compatible vector and matrix types.

#include "world/transform.hpp"           // `xform::Transform` and `xform::modelMatrix()`.


namespace scn {

// TYPES
// One name per .obj file.
enum ModelID {
	// Outdoor models.
	GRASS       ,
	PINE        ,
	OAK         ,
	SHIELD      ,
	MOON        ,
	NAVI_WINGS  ,
	NAVI_BODY   ,
	HUT_OUT     ,
	
	// Indoor models.
	HUT_IN      ,
	MAN         ,
	CANDLE      ,
	CANDLE_FLAME,
	POT         ,
	HEART_SHELL ,
	HEART_CORE  ,
	
	// Since each value of an `enum` is just an integer (starting from 0),
	// if we add an extra value at the end, it will hold the number of models
	// being used. Very smart!
	MODEL_COUNT
};

enum LampID {
	// Outdoor lights.
	NAVI_LAMP  ,
	
	// Indoor lights.
	CANDLE_LAMP,
	HEART_LAMP ,
	
	LAMP_COUNT
};
// `static_assert` checks a boolean constant expression at compile time.
static_assert(scn::LAMP_COUNT < 32, "More than 31 lamps defined.");

struct Scene {
	std::vector<glm::mat4> statics[scn::MODEL_COUNT]                  ;  // e.g.:                                                                
	                                                                     //       statics[GRASS]  →  [ M ]                   1 copy              
	                                                                     //       statics[PINE ]  →  [ M, M, M, M, ..., M ]  9 copies            
	                                                                     //       statics[OAK  ]  →  [ M, M, M ]             3 copies            
	                                                                     //       statics[NAVI ]  →  [ ]                     empty (it's dynamic)
	                                                                     //                      ...                                             
	
	xform::Transform       navi, flame, heart                         ;  // Their M is rebuilt every frame.
	
	bool                   ambientOn    = true                        ;
	std::uint32_t          lampsOn      = (1u << scn::LAMP_COUNT) - 1u;
	float                  ambientGain  = 1.0f                        ;
	float                  diffuseGain  = 1.0f                        ;
	float                  specularGain = 1.0f                        ;
};


// CONSTANTS
constexpr float TAU = 6.28318530718f;  // τ: one full turn, in radians.

// The model order below MUST mirror exactly that of `scn::ModelID`.
const char* const OBJ_PATHS[] = {
	"assets/grass/grass.obj"        ,
	"assets/pine/pine.obj"          ,
	"assets/oak/oak.obj"            ,
	"assets/shield/shield.obj"      ,
	"assets/moon/moon.obj"          ,
	"assets/navi/navi_wings.obj"    ,
	"assets/navi/navi_body.obj"     ,
	"assets/hut/hut_out.obj"        ,
	"assets/hut/hut_in.obj"         ,
	"assets/man/man.obj"            ,
	"assets/candle/candle.obj"      ,
	"assets/candle/candle_flame.obj",
	"assets/pot/pot.obj"            ,
	"assets/heart/heart_shell.obj"  ,
	"assets/heart/heart_core.obj"   ,
};
static_assert(std::size(OBJ_PATHS) == scn::MODEL_COUNT, "OBJ_PATHS must have one entry per Model ID.");

// Sky box faces, in OpenGL's cube-map conventional order: +X, -X, +Y, -Y, +Z, -Z.
const char* const SKYBOX_FACE_PATHS[6] = {
	"assets/skybox/right.png" ,
	"assets/skybox/left.png"  ,
	"assets/skybox/top.png"   ,
	"assets/skybox/bottom.png",
	"assets/skybox/front.png" ,
	"assets/skybox/back.png"  ,
};

constexpr glm::vec3 AMBIENT_LIGHT    = 0.2f * glm::vec3(0.77f, 0.72f, 1.0f);
// The light order of `LAMP_COLORS[]` MUST mirror exactly that of `scn::LampID`.
constexpr glm::vec3 LAMP_COLORS[] = {
	glm::vec3(0.7f, 0.8f, 1.0f),  // NAVI  : blue  .
	glm::vec3(1.0f, 0.6f, 0.3f),  // CANDLE: orange.
	glm::vec3(0.1f, 0.5f, 0.8f),  // HEART : azure .
};
static_assert(std::size(LAMP_COLORS) == scn::LAMP_COUNT, "LAMP_COLORS must have one entry per Lamp ID.");

constexpr float LAMP_HALF_DISTS[] = {
	5.0f,  // NAVI
	1.3f,  // CANDLE
	2.5f,  // HEART
};
static_assert(std::size(LAMP_HALF_DISTS) == scn::LAMP_COUNT, "LAMP_HALF_DISTS must have one entry per Lamp ID.");

constexpr std::uint32_t OUTDOOR_LAMPS = (1u << scn::NAVI_LAMP);
constexpr std::uint32_t INDOOR_LAMPS  = (1u << scn::CANDLE_LAMP) | (1u << scn::HEART_LAMP);

// The model order below MUST mirror exactly that of `scn::ModelID`.
constexpr std::uint32_t MODEL_LAMP_MASKS[] = {
	// Outdoor models.
	scn::OUTDOOR_LAMPS,  // GRASS
	scn::OUTDOOR_LAMPS,  // PINE
	scn::OUTDOOR_LAMPS,  // OAK
	scn::OUTDOOR_LAMPS,  // SHIELD
	scn::OUTDOOR_LAMPS,  // MOON
	scn::OUTDOOR_LAMPS,  // NAVI   (Wings   )
	scn::OUTDOOR_LAMPS,  // NAVI   (Body    )
	scn::OUTDOOR_LAMPS,  // HUT    (Outdoors)
	// Indoor models.
	scn::INDOOR_LAMPS ,  // HUT    (Indoors )
	scn::INDOOR_LAMPS ,  // MAN
	scn::INDOOR_LAMPS ,  // CANDLE (Body    )
	scn::INDOOR_LAMPS ,  // CANDLE (Flame   )
	scn::INDOOR_LAMPS ,  // POT
	scn::INDOOR_LAMPS ,  // HEART  (Shell   )
	scn::INDOOR_LAMPS ,  // HEART  (Core    )
};
static_assert(std::size(MODEL_LAMP_MASKS) == scn::MODEL_COUNT, "MODEL_LAMP_MASKS must have one entry per Model ID.");

constexpr glm::vec3 EYE_MIN    = glm::vec3(-20.0f,  0.3f, -20.0f);
constexpr glm::vec3 EYE_MAX    = glm::vec3( 20.0f, 30.0f,  20.0f);
constexpr glm::vec3 EYE_START  = glm::vec3(  0.0f,  1.7f,   8.0f);
constexpr glm::vec3 TRGT_START = glm::vec3(  0.0f,  1.0f,   0.0f);

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
	scn::place(scene, scn::GRASS  , {  0.0f,  0.0f ,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.5f  ));
	
	scn::place(scene, scn::PINE   , { -9.0f,  0.0f , -12.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.6f  ));
	scn::place(scene, scn::PINE   , { -5.0f,  0.0f ,  -9.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.5f  ));
	scn::place(scene, scn::PINE   , {  5.0f,  0.0f ,  -7.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(3.0f  ));
	scn::place(scene, scn::PINE   , {  5.0f,  0.0f ,   7.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.1f  ));
	scn::place(scene, scn::PINE   , {  0.0f,  0.0f ,  14.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.2f  ));
	scn::place(scene, scn::PINE   , { 10.0f,  0.0f ,   9.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.5f  ));
	scn::place(scene, scn::PINE   , { -7.0f,  0.0f ,  13.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.7f  ));
	scn::place(scene, scn::PINE   , {-10.0f,  0.0f ,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(2.0f  ));
	scn::place(scene, scn::PINE   , {-11.0f,  0.0f ,   4.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.0f  ));
	
	scn::place(scene, scn::OAK    , {  6.0f,  0.0f ,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.2f  ));
	scn::place(scene, scn::OAK    , {  0.0f,  0.0f ,  -5.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.9f  ));
	scn::place(scene, scn::OAK    , { -5.0f,  0.0f ,   6.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.6f  ));
	
	scn::place(scene, scn::SHIELD , { -2.5f,  0.01f,   0.6f}, {-0.04f*TAU, -0.25f*TAU, 0.0f*TAU}, glm::vec3(0.2f  ));
	
	scn::place(scene, scn::MOON   , {-20.0f, 70.0f ,  0.0f }, { 0.2f *TAU,  0.25f*TAU, 0.0f*TAU}, glm::vec3(0.015f));
	
	// The hut's own floor is modeled at y = 0 too: lifted a hair, so it doesn't flicker against the grass.
	scn::place(scene, scn::HUT_OUT, {  0.0f,  0.05f,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.0f  ));
	
	
	// INTERNAL
	scn::place(scene, scn::HUT_IN , {  0.0f,  0.05f,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(1.0f  ));
	scn::place(scene, scn::MAN    , {  0.0f,  0.08f,   0.0f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.01f ));
	scn::place(scene, scn::CANDLE , { -1.5f,  0.08f,  -0.8f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.07f ));
	scn::place(scene, scn::POT    , {  1.8f,  0.08f,  -0.8f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.03f ));
	scn::place(scene, scn::POT    , {  1.8f,  0.08f,   0.6f}, { 0.0f *TAU,  0.0f *TAU, 0.0f*TAU}, glm::vec3(0.03f ));
	
	
	// --- DYNAMIC ---
	// Only their starting values.
	//            position                , angles (radians)                              , scale
	
	scene.navi  = { {-2.0f ,  1.0f ,  4.0f }, glm::vec3(0.0f*TAU,  0.0f *TAU, 0.0f*TAU), glm::vec3(0.05f ) };
	
	scene.flame = { {-1.51f,  0.34f, -0.81f}, glm::vec3(0.0f*TAU,  0.0f *TAU, 0.0f*TAU), glm::vec3(0.07f ) };
	
	scene.heart = { { 1.8f ,  0.4f , -0.1f }, glm::vec3(0.0f*TAU, 0.25f *TAU, 0.0f*TAU), glm::vec3(0.03f ) };
}

}  // namespace scn
