#version 420 core

in vec2 vUV;                      // vert                u, v 
in vec3 vNormal;                  // vert norm.          x, y, z
in vec3 vViewDir;                 // vert view vector    x, y, z

out vec4 oColor;                  // out                 color

uniform vec3 uDiffuseCol;         // diffuse             color
uniform sampler2D uDiffuseMap;    // diffuse map         texture 
uniform vec3 uSpecularCol;        // specular            color
uniform sampler2D uSpecularMap;   // specular map        texture 
uniform float uShininess;         // shininess value     float
uniform sampler2D uShininessMap;  // shininess map       texture 
uniform vec3 uSubsurfCol;         // subsurface          color

uniform vec3 uSunDir;             // sun direction       x, y, z
uniform vec3 uSunCol;             // sun tint            color
uniform vec3 uAmbientCol;         // ambient tint        color
uniform sampler2D uAmbientMap;    // ambient map         texture

uniform bool uHasAmbientMap;      // ambient map flag    bool
uniform bool uHasDiffuseMap;      // diffuse map flag    bool
uniform bool uHasSpecularMap;     // specular map flag   bool
uniform bool uHasShininessMap;    // shininess map flag  bool

// diffuse ambient lightning mip level
#define DIFFUSE_MIP 7

// cubemap reflection intensity
#define SPECULAR_INTENSITY 0.5

// exponent of fresnel term
#define FRESNEL_EXPONENT 1.0

// subsurface term wrapping 
#define SUBSURFACE_WRAP 0.3

// exponent of subsurface term 
#define SUBSURFACE_EXPONENT 2.0

// hair clip threshold
#define CLIP_THRESHOLD 0.1

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

void main() {
	// normalize vectors
	vec3 N = normalize(vNormal);
	vec3 L = normalize(-uSunDir);
	vec3 V = normalize(vViewDir);
	vec3 R = reflect(-V, N);

	// get diffuse color
	vec3 albedo = uDiffuseCol;
	if(uHasDiffuseMap) {
		vec4 mapCol = texture(uDiffuseMap, vUV);
		albedo *= mapCol.rgb;
	
		// clip transparent hair
		if(mapCol.a < CLIP_THRESHOLD) discard; 
	}
	
	// get specular color
	vec3 specularCol = uSpecularCol;
	if(uHasSpecularMap) specularCol *= texture(uSpecularMap, vUV).rgb;

	// get shininess
	float shininess = uShininess;
	if(uHasShininessMap) shininess *= texture(uShininessMap, vUV).r;
	
	// remap shininess
	shininess = clamp(shininess, 0.0, 1.0);
	float specExponent = mix(1.0, 256.0, shininess * shininess);
	
	// calculate lambertian terms
	float lambertDot = dot(N, L);
	vec3 lambert = max(lambertDot, 0.0) * uSunCol;

	// calculate subsurface scattering
	float scatter = max(0.0, (lambertDot + SUBSURFACE_WRAP) / (1.0 + SUBSURFACE_WRAP)); 
	scatter = pow(1.0 - scatter, SUBSURFACE_EXPONENT);
	vec3 subsurf = scatter * uSubsurfCol * uSunCol;

	// calculate diffuse 
	vec3 diffuse = albedo * lambert;

	// get ambient
	vec3 enviro = uAmbientCol;
	if(uHasAmbientMap) {
		vec2 enviroUV = dirToEquirectUV(N);
		enviro *= textureLod(uAmbientMap, enviroUV, DIFFUSE_MIP).rgb;
	}
	vec3 ambient = enviro * albedo;

	// calculate specular
	vec3 oilSpecular =
		specularCol           *
		uSunCol               *
		step(0.0, lambertDot) *
		pow(max(dot(R, L), 0.0), specExponent);
	vec3 skinSpecular =
		specularCol           *
		uSunCol               *
		step(0.0, lambertDot) *
		pow(max(dot(R, L), 0.0), specExponent / 8);

	// calculate frag color
	vec3 color = 
		ambient          +
		subsurf          +
		diffuse          +
		oilSpecular  / 4;
		skinSpecular / 2;

	// final saturate
	oColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
