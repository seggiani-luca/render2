#version 420 core

layout(location = 0) in vec2 aPos;    // (0..1) quad
layout(location = 1) in vec4 iBounds; // x, y, w, h
layout(location = 2) in vec4 iUV;     // u0, v0, u1, v1

out vec2 vUV;

uniform vec2 uScreenSize; // window size in pixels

void main()
{
	// calculate position
	vec2 pos = iBounds.xy + aPos * iBounds.zw;

	// position to screen coordinates
	vec2 ndc = (pos / uScreenSize) * 2.0 - 1.0;
	ndc.y = -ndc.y;
	gl_Position = vec4(ndc, 0.0, 1.0);

	// calculate UVs
	vUV = mix(iUV.xy, iUV.zw, aPos);
}
