#version 330 core

layout (location = 0) in vec3 atr_Position;
layout (location = 1) in vec2 atr_TextureCoord;

uniform mat4 u_MatProjection = mat4(1);
uniform mat4 u_MatView = mat4(1);
uniform mat4 u_MatModel = mat4(1);

out vec2 v_TextureCoord;
out vec4 v_Position;

void main() {
	gl_Position = v_Position = u_MatProjection * u_MatView * u_MatModel * vec4(atr_Position, 1.0);
	v_TextureCoord = atr_TextureCoord;
}
