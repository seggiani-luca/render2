// cubemap reflection intensity
#define SPECULAR_INTENSITY 0.5

// exponent of fresnel term
#define FRESNEL_EXPONENT 1.0

// diffuse ambient lightning mip level
#define DIFFUSE_MIP 7

// fresnel factor between view direction and normal
float fresnel(vec3 N, vec3 V) {
	return pow(1 - dot(N, V), FRESNEL_EXPONENT);
}

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
