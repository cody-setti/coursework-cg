// --- MODEL FRAGMENT SHADER ---
// Runs once per fragment: decides WHAT COLOR the fragment is, or whether it is drawn at all.

#version 460 core

const int MAX_POINT_LIGHTS = 8;  // OBS.: Must match `MAX_POINT_LIGHTS` in "src/render/model.hpp".

                                                       in      vec3     varyingPos                       ;  // World space.
                                                       in      vec2     varyingST                        ;
                                                       in      vec3     varyingNrml                      ;  // World space.

// World    uniforms.
layout (        location = 1                         ) uniform vec3     ambientLight                     ;
layout (        location = 2                         ) uniform int      pLightCount                      ;
layout (        location = 3                         ) uniform vec3     pLightsPos     [MAX_POINT_LIGHTS];
layout (        location = 3 +   MAX_POINT_LIGHTS    ) uniform vec3     pLightsColor   [MAX_POINT_LIGHTS];
layout (        location = 3 + 2*MAX_POINT_LIGHTS    ) uniform float    pLightsHalfDist[MAX_POINT_LIGHTS];
layout (std140, binding  = 0                         ) uniform Camera   { mat4 V; mat4 P; vec3 eye; } ;

// Material uniforms.
layout (        binding  = 0                         ) uniform sampler2D colorMap                        ;  // Reads from texture unit 0.
layout (        location = 3 + 3*MAX_POINT_LIGHTS    ) uniform vec3      Kd                              ;
layout (        location = 3 + 3*MAX_POINT_LIGHTS + 1) uniform vec3      Ks                              ;
layout (        location = 3 + 3*MAX_POINT_LIGHTS + 2) uniform float     Ns                              ;
layout (        location = 3 + 3*MAX_POINT_LIGHTS + 3) uniform vec3      Ke                              ;

layout (        location = 3 + 3*MAX_POINT_LIGHTS + 4) uniform uint      pModelMask                      ;
layout (        location = 3 + 3*MAX_POINT_LIGHTS + 5) uniform uint      pLightsOn                       ;
layout (        location = 3 + 3*MAX_POINT_LIGHTS + 6) uniform float     ambientScale                    ;
layout (        location = 3 + 3*MAX_POINT_LIGHTS + 7) uniform float     diffuseScale                    ;
layout (        location = 3 + 3*MAX_POINT_LIGHTS + 8) uniform float     specularScale                   ;

                                                       out     vec4      fragColor                       ;

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
	
	for ( int i = 0; i < pLightCount; i++ ) {
		if ( (pModelMask & pLightsOn & (1u << i)) == 0u ) {
			continue;  // Light `i` is off, or doesn't reach this model.
		}
		const vec3  toLight = pLightsPos[i] - varyingPos;
		const float d       = length(toLight);
		
		const vec3  L     = normalize(toLight);
		const float cosNL = dot(N, L);
		
		if ( cosNL > 0.0 ) {
			// ATTENUATION
			const float h           = pLightsHalfDist[i]           ;
			const float attenuation = 1.0 / (1.0 + (d*d)/(h*h))    ;
			const vec3  arriving    = attenuation * pLightsColor[i];
			
			
			diffuseLight  += arriving * cosNL;
			
			const vec3 R = reflect(-L, N);
			specularLight += arriving * pow( max(dot(R, E), 0.0), max(Ns, 0.001) );
			//                       The max's avoid:  negative bases   and   0 ** 0.
		}
	}
	
	
	
	// REFLECTED LIGHT
	const vec3 ambient  = ambientScale  * ambientLight  * Kd * color.rgb;
	const vec3 diffuse  = diffuseScale  * diffuseLight  * Kd * color.rgb;
	const vec3 specular = specularScale * specularLight * Ks            ;
	// OBS.: vector/vector multiplication is COMPONENT-WISE in GLSL.
	
	
	
	// EMITTED LIGHT
	const vec3 emitted = Ke * color.rgb;
	
	
	
	fragColor = vec4(ambient + diffuse + specular + emitted, color.a);
}
