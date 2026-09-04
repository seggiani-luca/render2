#version 420 core

in vec2 vUV;             // vert   u, v

out vec4 oColor;         // out    color

uniform sampler2D uTex;  // atlas  texture

void main()
{
	// sample texture
    oColor = texture(uTex, vUV);
}
