#version 420 core

layout(location = 0) in vec3 aPos;  // vertex         x, y, z

out vec3 skyDir;                    // out sky vector x, y, z

uniform mat4 uView;                 // view           matrix
uniform mat4 uProjection;           // projection     matrix

void main() {
	// position is sky vector 
	skyDir = aPos;

	// output position, full depth
	gl_Position = (uProjection * uView * vec4(aPos, 1.0)).xyww;
}
