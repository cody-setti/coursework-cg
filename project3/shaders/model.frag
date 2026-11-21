// --- MODEL FRAGMENT SHADER ---
// Runs once per fragment: decides WHAT COLOR the fragment is, or whether it is drawn at all.

#version 460 core

                              in      vec3      varyingPos                  ;  // World space.
                              in      vec2      varyingST                   ;
                              in      vec3      varyingNrml                 ;  // World space.
layout (std140, binding  = 0) uniform Camera    { mat4 V; mat4 P; vec3 eye;};
layout (        binding  = 0) uniform sampler2D colorMap                    ;  // Reads from texture unit 0.
layout (        location = 1) uniform vec3      Kd                          ;
layout (        location = 2) uniform vec3      Ks                          ;
layout (        location = 3) uniform float     Ns                          ;
layout (        location = 4) uniform vec3      ambientLight                ;
layout (        location = 5) uniform vec3      lightPos                    ;
layout (        location = 6) uniform vec3      lightColor                  ;
                              out     vec4      fragColor                   ;

void main() {
	const vec4 color = texture(colorMap, varyingST);  // Color of the texture at this fragment's (s, t).
	
	// ALPHA CUTOUT: drop mostly transparent fragments so they write neither color nor depth.
	// Specifically, we use this for the pine and oak model's leaves.
	if ( color.a < 0.1 ) {
		discard;
	}
	
	// UNIT VECTORS, all leaving the fragment.
	vec3 N = normalize(varyingNrml          );  // Interpolation (and M's scale) shorten the normal: restore length 1.
	if ( !gl_FrontFacing ) {
		N = -N;  // The leaves and navi are single sided!
	}
	const vec3 E = normalize(     eye - varyingPos);
	const vec3 L = normalize(lightPos - varyingPos);
	const vec3 R = reflect(-L, N);
	
	const float cosNL = dot(N, L);
	
	const vec3 ambient  = ambientLight * Kd * color.rgb;
	const vec3 diffuse  = lightColor   * Kd * color.rgb * max(cosNL, 0.0);
	      vec3 specular = vec3(0.0);
	if ( cosNL > 0.0 ) {
		specular = lightColor * Ks * pow(max(dot(R, E), 0.0), max(Ns, 0.001));
	}
	
	fragColor = vec4(ambient + diffuse + specular, color.a);
}
