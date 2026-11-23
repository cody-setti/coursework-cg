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

enum PointLightID {
	// Outdoor lights.
	POINT_LIGHT_NAVI  ,
	
	// Indoor lights.
	POINT_LIGHT_CANDLE,
	POINT_LIGHT_HEART ,
	
	POINT_LIGHT_COUNT
};

struct Scene {
	std::vector<glm::mat4> statics[scn::MODEL_COUNT];  // e.g.:                                                                
	                                                   //       statics[GRASS]  →  [ M ]                   1 copy              
	                                                   //       statics[PINE ]  →  [ M, M, M, M, ..., M ]  9 copies            
	                                                   //       statics[OAK  ]  →  [ M, M, M ]             3 copies            
	                                                   //       statics[NAVI ]  →  [ ]                     empty (it's dynamic)
	                                                   //                      ...                                             
	
	xform::Transform       navi, flame, heart       ;  // Their M is rebuilt every frame.
	
	bool     ambientOn     = true                               ;
	unsigned pLightsOn     = (1u << scn::POINT_LIGHT_COUNT) - 1u;
	float    ambientScale  = 1.0f                               ;
	float    diffuseScale  = 1.0f                               ;
	float    specularScale = 1.0f                               ;
};


// CONSTANTS
// TODO: reflect if constants order is ideal and if the ones that depend on ModelID should be write next to it
//       breaking the usual TYPES/CONSTANTS/FUNCTIONS split.
constexpr float TAU = 6.28318530718f;  // τ: one full turn, in radians.

// The model order below MUST mirror exactly that of `scn::ModelID`.
const char* const OBJ_PATHS[scn::MODEL_COUNT] = {
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
// The light order of `POINT_LIGHTS_COLOR[]` MUST mirror exactly that of `scn::PointLightID`.
constexpr glm::vec3 POINT_LIGHTS_COLOR[scn::POINT_LIGHT_COUNT] = {
	glm::vec3(0.7f, 0.8f, 1.0f),  // NAVI  : blue  .
	glm::vec3(1.0f, 0.6f, 0.3f),  // CANDLE: orange.
	glm::vec3(0.1f, 0.5f, 0.8f),  // HEART : azure .
};

constexpr float POINT_LIGHTS_HALF_DIST[scn::POINT_LIGHT_COUNT] = {
	5.0f,  // NAVI
	1.3f,  // CANDLE
	2.5f,  // HEART
};

constexpr unsigned OUTDOOR_PLIGHTS = (1u << scn::POINT_LIGHT_NAVI);
constexpr unsigned INDOOR_PLIGHTS  = (1u << scn::POINT_LIGHT_CANDLE) | (1u << scn::POINT_LIGHT_HEART);

// The model order below MUST mirror exactly that of `scn::ModelID`.
constexpr unsigned MODEL_PLIGHTS_MASKS[scn::MODEL_COUNT] = {
	// Outdoor models.
	scn::OUTDOOR_PLIGHTS,  // GRASS
	scn::OUTDOOR_PLIGHTS,  // PINE
	scn::OUTDOOR_PLIGHTS,  // OAK
	scn::OUTDOOR_PLIGHTS,  // SHIELD
	scn::OUTDOOR_PLIGHTS,  // MOON
	scn::OUTDOOR_PLIGHTS,  // NAVI   (Wings   )
	scn::OUTDOOR_PLIGHTS,  // NAVI   (Body    )
	scn::OUTDOOR_PLIGHTS,  // HUT    (Outdoors)
	// Indoor models.
	scn::INDOOR_PLIGHTS,   // HUT    (Indoors )
	scn::INDOOR_PLIGHTS,   // MAN
	scn::INDOOR_PLIGHTS,   // CANDLE (Body    )
	scn::INDOOR_PLIGHTS,   // CANDLE (Flame   )
	scn::INDOOR_PLIGHTS,   // POT
	scn::INDOOR_PLIGHTS,   // HEART  (Shell   )
	scn::INDOOR_PLIGHTS,   // HEART  (Core    )
};

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
