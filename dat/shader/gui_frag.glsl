#version 420 core

in vec2 vUV; // atlas uv

out vec4 FragColor;

uniform sampler2D uTex; // atlas texture

void main()
{
	// sample texture
    FragColor = texture(uTex, vUV);
}
