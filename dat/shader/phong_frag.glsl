#version 420 core

in vec2 vUV;                         // vert               u, v 
in vec3 vNormal;                     // vert norm.         x, y, z
in vec3 vViewDir;                    // vert view vector   x, y, z

out vec4 oColor;                     // out                color

uniform vec3 uDiffuseCol;            // diffuse            color
uniform sampler2D uDiffuseMap;       // diffuse map        texture 
uniform vec3 uSpecularCol;           // specular           color
uniform sampler2D uSpecularMap;      // specular map       texture 
uniform float uShininess;            // shininess value    float

uniform vec3 uSunDir;                // sun direction      x, y, z
uniform vec3 uSunCol;                // sun tint           color
uniform vec3 uAmbientCol;            // ambient tint       color

uniform bool uHasDiffuseMap;         // diffuse map flag   bool
uniform bool uHasSpecularMap;        // specular map flag  bool

void main() {
	vec3 N = normalize(vNormal);
	vec3 L = normalize(-uSunDir);
	vec3 V = normalize(vViewDir);

	// get ambient
	vec3 ambient = uAmbientCol;

	// get diffuse color
	vec3 albedo = uDiffuseCol;
	if(uHasDiffuseMap) albedo *= texture(uDiffuseMap, vUV).rgb;

	// calculate lambertian terms
	float lambertDot = dot(L, N);
	vec3 lambert = max(lambertDot, 0.0) * uSunCol;

	// calculate diffuse 
	vec3 diffuse = albedo * lambert;

	// get specular color
	vec3 specularCol = uSpecularCol;
	if(uHasSpecularMap) specularCol *= texture(uSpecularMap, vUV).rgb;

	// calcolate specular
	vec3 halfway = normalize(L + V);
	vec3 specular =
		specularCol           *
		uSunCol               *
		step(0.0, lambertDot) *
		pow(max(dot(N, halfway), 0.0), uShininess);

	// calculate frag color
	vec3 color = 
		ambient +
		diffuse +
		specular;

	// final saturate
	oColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
