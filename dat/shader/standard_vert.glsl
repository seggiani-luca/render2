#version 420 core

layout(location = 0) in vec3 aPos;     // vertex           x, y, z
layout(location = 1) in vec2 aUV;      // vertex           u, v
layout(location = 2) in vec3 aNormal;  // vertex normal    x, y, z

out vec2 vUV;                          // out              u, v 
out vec3 vNormal;                      // out normal       x, y, z
out vec3 vViewDir;                     // out view vector  x, y, z

uniform mat4 uModel;                   // model transform  matrix 

uniform mat4 uView;                    // view             matrix
uniform mat4 uProjection;              // projection       matrix
uniform vec3 uCameraPos;               // camera position  x, y, z

void main() {
	// calculate world position
	vec3 worldPos = vec3(uModel * vec4(aPos, 1.0));

	// calculate projected position
	gl_Position = uProjection * uView * vec4(worldPos, 1.0);

	// pass over other parameters
	vUV = aUV;
	vNormal = mat3(transpose(inverse(uModel))) * aNormal;
	vViewDir = uCameraPos - worldPos;
}
