#version 330
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;

uniform float time;

out vec2 UV;

void main()
{
    vec3 position = vertexPosition;
    
//    position.x = position.x + (sin(position.y * time * 8) * 0.08);
    
    gl_Position.xyz = position;
    gl_Position.w = 1.0;
    
    UV = vertexUV;
}
