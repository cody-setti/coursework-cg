// --- MODEL FRAGMENT SHADER ---
// Runs once per fragment: decides WHAT COLOR the fragment is, or whether it is drawn at all.

#version 460 core

                                      in      vec3      varyingPos                  ;  // World space.
                                      in      vec2      varyingST                   ;
                                      in      vec3      varyingNrml                 ;  // World space.

// Model    uniform.
layout (        location = 1        ) uniform uint      modelLamps                  ;  // Model's lamps that are SWITCHED ON.

// Material uniforms.
layout (        binding  = 0        ) uniform sampler2D colorMap                    ;  // Reads from texture unit 0.
layout (        location = 2        ) uniform vec3      Kd                          ;
layout (        location = 3        ) uniform vec3      Ks                          ;
layout (        location = 4        ) uniform float     Ns                          ;
layout (        location = 5        ) uniform vec3      Ke                          ;

// World    uniforms.

layout (std140, binding  = 0        ) uniform Camera   { mat4 V; mat4 P; vec3 eye; };
layout (        location = 6        ) uniform float     ambientGain                 ;
layout (        location = 7        ) uniform float     diffuseGain                 ;
layout (        location = 8        ) uniform float     specularGain                ;

layout (        location = 9        ) uniform vec3      ambientLight                ;

layout (        location = 10       ) uniform vec3      lampPositions[32]           ;
layout (        location = 10 +   32) uniform vec3      lampColors   [32]           ;
layout (        location = 10 + 2*32) uniform float     lampHalfDists[32]           ;

                                      out     vec4      fragColor                   ;

void main() {
	const vec4 color = texture(colorMap, varyingST);  // Color of the texture at this fragment's (s, t).
	
	// ALPHA CUTOUT: drop mostly transparent fragments so they write neither color nor depth.
	// Specifically, we use this for the pine and oak model's leaves.
	if ( color.a < 0.1 ) {
		discard;
	}
	
	
	
	vec3 N = normalize(varyingNrml);  // Interpolation (and M's scale) shorten the normal: restore length 1.
	if ( !gl_FrontFacing ) {
		N = -N;  // Make all surfaces point to the eye.
		         // The depth test prevents this flip from affecting the back faces of closed meshes.
		         // It serves the leaves and Navi, which are single-sided meshes.
	}
	const vec3 E = normalize(eye - varyingPos);
	
	
	
	// INCIDENT LIGHT
	vec3 diffuseLight  = vec3(0.0);
	vec3 specularLight = vec3(0.0);
	
	uint lit = modelLamps;
	while ( lit != 0u) {
		const int i = findLSB(lit);  // Index of the lowest bit set to 1: the next lamp to light with.
		lit ^= (1u << i);            // Bit i is known to be 1, so XOR clears it.
		
		const vec3  toLight = lampPositions[i] - varyingPos;
		const float d       = length(toLight);
		
		const vec3  L     = normalize(toLight);
		const float cosNL = dot(N, L);
		
		if ( cosNL > 0.0 ) {
			// ATTENUATION
			const float h           = lampHalfDists[i]           ;
			const float attenuation = 1.0 / (1.0 + (d*d)/(h*h))  ;
			const vec3  arriving    = attenuation * lampColors[i];
			
			
			diffuseLight  += arriving * cosNL;
			
			const vec3 R = reflect(-L, N);
			specularLight += arriving * pow( max(dot(R, E), 0.0), max(Ns, 0.001) );
			//                       The max's avoid:  negative bases   and   0 ** 0.
		}
	}
	
	
	
	// REFLECTED LIGHT
	const vec3 ambient  = ambientGain  * ambientLight  * Kd * color.rgb;
	const vec3 diffuse  = diffuseGain  * diffuseLight  * Kd * color.rgb;
	const vec3 specular = specularGain * specularLight * Ks            ;
	// OBS.: vector/vector multiplication is COMPONENT-WISE in GLSL.
	
	
	
	// EMITTED LIGHT
	const vec3 emitted = Ke * color.rgb;
	
	
	
	fragColor = vec4(ambient + diffuse + specular + emitted, color.a);
}
