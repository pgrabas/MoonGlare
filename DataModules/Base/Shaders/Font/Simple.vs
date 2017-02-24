#include "Common.glsl"

 uniform sampler2D Texture0;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextureUV;
layout(location = 2) in vec3 Normal;

out vec2 VertexUV0;
out vec3 VertexNormal;
out vec4 VertexWorldPos;
out vec4 VertexPosition;

void main() {
	vec4 vpos = (CameraMatrix * ModelMatrix) * vec4(Position, 1.0);

	gl_Position = vpos;
	VertexPosition = vpos;
    VertexUV0 = TextureUV;
	VertexNormal = (ModelMatrix * vec4(Normal, 0.0)).xyz;
	VertexWorldPos = (ModelMatrix * vec4(Position, 1.0));
};
