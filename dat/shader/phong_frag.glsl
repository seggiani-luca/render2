#version 420 core

in vec2 vUV;                         // vert                u, v 
in vec3 vNormal;                     // vert norm.          x, y, z
in vec3 vViewDir;                    // vert view vector    x, y, z

out vec4 oColor;                     // out                 color

uniform vec3 uDiffuseCol;            // diffuse             color
uniform sampler2D uDiffuseMap;       // diffuse map         texture 
uniform vec3 uSpecularCol;           // specular            color
uniform sampler2D uSpecularMap;      // specular map        texture 
uniform float uShininess;            // shininess value     float
uniform sampler2D uShininessMap;     // shininess map       texture 

uniform vec3 uSunDir;                // sun direction       x, y, z
uniform vec3 uSunCol;                // sun tint            color
uniform vec3 uAmbientCol;            // ambient tint        color
uniform sampler2D uAmbientMap;       // ambient map         texture

uniform bool uHasAmbientMap;         // ambient map flag    bool
uniform bool uHasDiffuseMap;         // diffuse map flag    bool
uniform bool uHasSpecularMap;        // specular map flag   bool
uniform bool uHasShininessMap;       // shininess map flag  bool

// diffuse ambient lightning mip level
#define DIFFUSE_MIP 7

// cubemap reflection intensity
#define SPECULAR_INTENSITY 0.5

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
	if(uHasDiffuseMap) albedo *= texture(uDiffuseMap, vUV).rgb;
	
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
	float lambertDot = dot(L, N);
	vec3 lambert = max(lambertDot, 0.0) * uSunCol;

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
	vec3 halfway = normalize(L + V);
	vec3 specular =
		specularCol           *
		uSunCol               *
		step(0.0, lambertDot) *
		pow(max(dot(N, halfway), 0.0), specExponent);
	if(uHasAmbientMap) {
		vec2 specUV = dirToEquirectUV(R);
		float specMip = (1.0 - shininess) * DIFFUSE_MIP;
		specular += 
			textureLod(uAmbientMap, specUV, specMip).rgb *
			specularCol                                  *
			shininess                                    *
			SPECULAR_INTENSITY;
	}

	// calculate frag color
	vec3 color = 
		ambient +
		diffuse +
		specular;

	// final saturate
	oColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
