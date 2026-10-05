#pragma once
// --- WAVEFRONT PARSERS ---
// Parsing Wavefront .obj models and their .mtl materials into plain C++ data (no OpenGL involved).
#include <vector>                         // The `std::vector` object is used to manage vertex data.
#include <string>                         // The `std::string` object is used to manage .obj and .mtl lines.
#include <sstream>                        // String       Stream: parse text from memory  using `std::stringstream`.
#include <fstream>                        // File         Stream: read (.obj, .mtl) files using `std::ifstream`.
#include <iostream>                       // Input/Output Stream: output error messages   using `std::cerr`.
#include <map>                            // The `std::map` object is used to look up image paths by material name.

#include <glm/glm.hpp>                    // GL Mathematics: `glm::vec3` and `glm::vec2`.


namespace my {

// Each vertex in the VBO is stored as x, y, z (position) followed by s, t (texture coordinates).
constexpr int FLOATS_PER_VERTEX = 5;

// Returns the folder part of a file path, including the trailing '/' (e.g.: "assets/hut/hut.obj" -> "assets/hut/").
inline std::string directoryOf(const std::string& path) {
	return path.substr(0, path.find_last_of('/') + 1);
	// OBS.: If there is no '/', `find_last_of()` returns `std::string::npos` (the largest possible value),
	//       and `npos + 1` wraps around to 0, so the result is "".
}

// A slice of the VBO associated with a particular texture material.
struct MaterialRange {
	std::string material;  // Name given by the `usemtl` line that started the slice.
	int         first   ;  // Index of the slice's first vertex.
	int         count   ;  // Number of vertices in the slice.
};

// The contents of a parsed .obj file: vertex data arranged by its texture materials.
struct ParsedOBJ {
	// Flat list of triangle vertices:
	// { x1,y1,z1,s1,t1, x2,y2,z2,s2,t2, x3,y3,z3,s3,t3, ... }.
	std::vector<float>             vertexData;
	
	// One range per `usemtl` line in file order.
	std::vector<my::MaterialRange> ranges    ;
	
	// Path to the .mtl file named by the `mtllib` line ("" if there is none).
	std::string                    mtlPath   ;
};

// Parses a Wavefront .obj file into a `my::ParsedOBJ` struct. Returns an empty struct on failure.
// OBS.: Only `v`, `vt`, `f`, `usemtl` and `mtllib` lines are read for now, and every face is assumed to be a triangle.
inline my::ParsedOBJ parseOBJ(const char* path) {
	std::ifstream file(path);
	if ( !file ) {
		std::cerr << "Could not open file: " << path << std::endl;
		return {};
	}
	
	
	 my::ParsedOBJ          parsed  ;
	std::string            line     ;
	std::vector<glm::vec3> positions;  // Every `v`  line, in file order.
	std::vector<glm::vec2> texCoords;  // Every `vt` line, in file order.
	
	// `std::getline(file, line)` reads characters from `file` up to the next `\n`, puts them in `line` (without the `\n`),
	// and moves the file's read position to the start of the next line.
	// It returns the stream `file` itself, and a stream can be used as a condition:
	// it's `true` while the last read succeeded and `false` once a read failed. Reading past the end of the file is a failure.
	while ( std::getline(file, line) ) {
		// Wrap `line` in a stream, so we can read through it piece by piece with `>>`.
		std::stringstream ss(line);
		
		std::string prefix;
		ss >> prefix;
		
		if ( prefix == "v" ) {
			glm::vec3 pos;
			ss >> pos.x >> pos.y >> pos.z;
			positions.push_back(pos);
		}
		else if ( prefix == "vt" ) {
			glm::vec2 st;
			ss >> st.s >> st.t;
			texCoords.push_back(st);
		}
		else if ( prefix == "mtllib" ) {
			// The .mtl file's name is relative to the .obj file's folder.
			std::string mtlFile;
			ss >> mtlFile;
			
			parsed.mtlPath = my::directoryOf(path) + mtlFile;
		}
		else if ( prefix == "usemtl" ) {
			// Start a new, still empty, `my::MaterialRange` at the current vertex.
			
			std::string material;
			ss >> material;
			
			int firstVertex = parsed.vertexData.size() / my::FLOATS_PER_VERTEX;
			
			parsed.ranges.push_back(
				{ material   , 
				  firstVertex, 
				  0           }
			);
		}
		else if ( prefix == "f" ) {
			// Faces that come before any `usemtl` get a range with no material name.
			if ( parsed.ranges.empty() ) {
				parsed.ranges.push_back( {"", 0, 0} );
			}
			
			for ( std::string corner; ss >> corner; ) {
				// Each corner looks like `v/vt/vn` (1-based vertex indices).
				// `std::stoi()` stops at the first '/', so it reads `v` from the start and `vt` from after the first '/'.
				glm::vec3 pos = positions[std::stoi(corner                             ) - 1];
				glm::vec2 st  = texCoords[std::stoi(corner.substr(corner.find('/') + 1)) - 1];
				
				parsed.vertexData.insert(parsed.vertexData.end(), {pos.x, pos.y, pos.z, st.s, st.t});
				parsed.ranges.back().count++;
			}
		}
	}
	return parsed;
}

// Reads a Wavefront .mtl file and finds each material's diffuse texture image (its `map_Kd` line).
// Returns a map from material name to image path, or an empty map on failure.
// OBS.: Only `newmtl` and `map_Kd` lines are read, and `map_Kd` options (e.g.: `-s 2 2 2`) are not supported.
inline std::map<std::string, std::string> parseMTL(const char* path) {
	std::ifstream file(path);
	if ( !file ) {
		std::cerr << "Could not open file: " << path << std::endl;
		return {};
	}
	
	std::map<std::string, std::string> imagePaths;
	std::string                        material  ;  // Name in the most recently read `newmtl` line.
	
	std::string line;
	
	while ( std::getline(file, line) ) {
		std::stringstream ss(line);
		
		std::string prefix;
		ss >> prefix;
		
		if ( prefix == "newmtl" ) {
			ss >> material;
		}
		else if ( prefix == "map_Kd" ) {
			// The image's name is relative to the .mtl file's folder.
			std::string imageFile;
			ss >> imageFile;
			
			imagePaths[material] = my::directoryOf(path) + imageFile;
		}
	}
	return imagePaths;
}

}  // namespace my
