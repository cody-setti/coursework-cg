// --- WAVEFRONT ---
// Parses Wavefront .obj models and their .mtl materials into plain C++ data (no OpenGL involved).

#pragma once                              // `#include` literally pastes a file's text in place.
                                          // `#pragma once` protects `wavefront.hpp` from being double pasted.

#include <vector>                         // The `std::vector` object is used to manage vertex data.
#include <string>                         // The `std::string` object is used to manage .obj and .mtl lines.
#include <sstream>                        // String       Stream: parse text from memory  using `std::stringstream`.
#include <fstream>                        // File         Stream: read (.obj, .mtl) files using `std::ifstream`.
#include <iostream>                       // Input/Output Stream: output error messages   using `std::cerr`.
#include <map>                            // The `std::map` object is used to look up parsed materials by their names.

#include <glm/glm.hpp>                    // GL Mathematics: `glm::vec3` and `glm::vec2`.


namespace wavef {

// TYPES
// A slice of the VBO associated with a particular texture material.
struct MaterialRange {
	std::string mtlName;  // Name given by the `usemtl` line that started the slice.
	int         first  ;  // Index of the slice's first vertex.
	int         count  ;  // Number of vertices in the slice.
};

// The contents of a parsed .obj file: vertex data arranged by its texture materials.
struct ParsedOBJ {
	// Flat list of triangle vertices:
	// { x1,y1,z1,s1,t1,nx1,ny1,nz1,  x2,y2,z2,s2,t2,nx2,ny2,nz2,  ... }.
	std::vector<float>                vertexData;
	
	// One range per `usemtl` line in file order.
	std::vector<wavef::MaterialRange> ranges    ;
	
	// Path to the .mtl file named by the `mtllib` line ("" if there is none).
	std::string                       mtlPath   ;
};

// The contents of one `newmtl` block of a parsed .mtl file.
// OBS.: The defaults stand in for lines the block leaves out.
struct ParsedMaterial {
	std::string imagePath ;  // `map_Kd`: path to the texture's image file.
	
	// `Kd` and `Ks` are 3D vectors because light is handled as three separate channels (RGB).
	glm::vec3   Kd  {1.0f};  // White (1.0f) leaves the texture's color untouched.
	glm::vec3   Ks  {0.0f};  // Black (0.0f) means no highlight.
	float       Ns = 1.0f ;  // The larger the specular exponent, the smaller and sharper the highlight.
};


// CONSTANTS
constexpr int FLOATS_PER_VERTEX = 8;  // 3 floats for position, 2 for texture and 3 for normals.


// FUNCTIONS
// Returns the folder part of a file path, including the trailing '/' (e.g.: "assets/hut/hut.obj" -> "assets/hut/").
inline std::string directoryOf(const std::string& path) {
	return path.substr(0, path.find_last_of('/') + 1);
	// OBS.: If there is no '/', `find_last_of()` returns `std::string::npos` (the largest possible value),
	//       and `npos + 1` wraps around to 0, so the result is "".
}

// Parses a Wavefront .obj file into a `wavef::ParsedOBJ` struct. Returns an empty struct on failure.
// OBS.: Every face is assumed to be a triangle.
inline wavef::ParsedOBJ parseOBJ(const char* path) {
	std::ifstream file(path);
	if ( !file ) {
		std::cerr << "Could not open file: " << path << std::endl;
		return {};
	}
	
	
	wavef::ParsedOBJ       parsed   ;
	std::string            line     ;
	std::vector<glm::vec3> positions;  // Every `v`  line, in file order.
	std::vector<glm::vec2> texCoords;  // Every `vt` line, in file order.
	std::vector<glm::vec3> normals  ;  // Every `vn` line, in file order.
	
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
		else if ( prefix == "vn" ) {
			glm::vec3 n;
			ss >> n.x >> n.y >> n.z;
			normals.push_back(n);
		}
		else if ( prefix == "mtllib" ) {
			// The .mtl file's name is relative to the .obj file's folder.
			std::string mtlFile;
			ss >> mtlFile;
			
			parsed.mtlPath = wavef::directoryOf(path) + mtlFile;
		}
		else if ( prefix == "usemtl" ) {
			// Start a new, still empty, `wavef::MaterialRange` at the current vertex.
			
			std::string mtlName;
			ss >> mtlName;
			
			const int firstVertex = parsed.vertexData.size() / wavef::FLOATS_PER_VERTEX;
			
			parsed.ranges.push_back( {mtlName, firstVertex, 0} );
		}
		else if ( prefix == "f" ) {
			// Faces that come before any `usemtl` get a range with no material name.
			if ( parsed.ranges.empty() ) {
				parsed.ranges.push_back( {"", 0, 0} );
			}
			
			for ( std::string corner; ss >> corner; ) {
				// Each corner looks like `v/vt/vn` (1-based vertex indices).
				// `std::stoi()` stops at the first `/`, so it reads `v` from the start,
				// `vt` from after the first `/` and `vn` from after the second `/`.
				const glm::vec3 pos = positions[std::stoi(corner                             ) - 1];
				const glm::vec2 st  = texCoords[std::stoi(corner.substr(corner. find('/') + 1)) - 1];
				const glm::vec3 n   = normals  [std::stoi(corner.substr(corner.rfind('/') + 1)) - 1];
				
				parsed.vertexData.insert(parsed.vertexData.end(), {pos.x, pos.y, pos.z, st.s, st.t, n.x, n.y, n.z});
				parsed.ranges.back().count++;
			}
		}
	}
	
	if ( parsed.vertexData.empty() ) {
		std::cerr << "No faces found in: " << path << std::endl;
	}
	
	return parsed;
}

// Reads a Wavefront .mtl file and finds each material's texture image (`map_Kd`) and reflection values (`Kd`, `Ks`, `Ns`).
// Returns a map from material name to its contents, or an empty map on failure.
// OBS.: `map_Kd` options (e.g.: `-s 2 2 2`) are not supported.
inline std::map<std::string, wavef::ParsedMaterial> parseMTL(const char* path) {
	std::ifstream file(path);
	if ( !file ) {
		std::cerr << "Could not open file: " << path << std::endl;
		return {};
	}
	
	std::map<std::string, wavef::ParsedMaterial> materials;
	std::string                                  name     ;  // Name in the most recently read `newmtl` line.
	
	std::string line;
	
	while ( std::getline(file, line) ) {
		std::stringstream ss(line);
		
		std::string prefix;
		ss >> prefix;
		
		if ( prefix == "newmtl" ) {
			ss >> name;
			
			materials[name] = {};
		}
		else if ( prefix == "map_Kd" ) {
			std::string imageFile;
			ss >> imageFile;
			
			materials[name].imagePath = wavef::directoryOf(path) + imageFile;
		}
		else if ( prefix == "Kd" ) {
			glm::vec3 Kd;
			ss >> Kd.r >> Kd.g >> Kd.b;
			
			materials[name].Kd = Kd;
		}
		else if ( prefix == "Ks" ) {
			glm::vec3 Ks;
			ss >> Ks.r >> Ks.g >> Ks.b;
			
			materials[name].Ks = Ks;
		}
		else if ( prefix == "Ns" ) {
			float Ns;
			ss >> Ns;
			
			materials[name].Ns = Ns;
		}
	}
	
	if ( materials.empty() ) {
		std::cerr << "No materials found in: " << path << std::endl;
	}
	
	return materials;
}

}  // namespace wavef
