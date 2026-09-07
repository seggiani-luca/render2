#version 420 core

in vec3 skyDir;                 // in sky vector     x, y, z

out vec4 oColor;                // out               color

uniform vec3 uDiffuseCol;       // diffuse           color
uniform sampler2D uDiffuseMap;  // diffuse map       texture 
uniform bool uHasDiffuseMap;    // diffuse map flag  bool

// converts vectors to equirectangular coordinates
vec2 dirToEquirectUV(vec3 dir) {
	const float PI = 3.14159265359;

	// equirectangular map
	float phi = atan(dir.z, dir.x);
	float theta = asin(dir.y);

	// rescale and return
	return vec2(
		phi / (2.0 * PI) + 0.5,
		theta / PI + 0.5
	);
}

void main() {
	if(!uHasDiffuseMap) discard;

	// convert equirectangular
	vec2 skyUV = dirToEquirectUV(normalize(skyDir));
	vec3 color = uDiffuseCol * texture(uDiffuseMap, skyUV).rgb;
	
	// final saturate
	oColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
