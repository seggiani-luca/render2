#version 420 core

#include "dat/shader/includes/includes_frag.glsl"

in vec3 skyDir;                 // in sky vector     x, y, z

out vec4 oColor;                // out               color

uniform vec3 uDiffuseCol;       // diffuse           color
uniform sampler2D uDiffuseMap;  // diffuse map       texture 
uniform bool uHasDiffuseMap;    // diffuse map flag  bool

void main() {
	if(!uHasDiffuseMap) discard;

	// convert equirectangular
	vec2 skyUV = dirToEquirectUV(normalize(skyDir));
	vec3 color = uDiffuseCol * texture(uDiffuseMap, skyUV).rgb;
	
	// final saturate
	oColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
