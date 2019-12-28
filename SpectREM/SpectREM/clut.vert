#version 330

// Layout of the vertex and UV data being passed in using an interleaved VBO
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;

// Variable that the UV coordinate is passed out too
out vec2 v_texCoord;

///////////////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////////////
void main()
{
    // Pass the UV read from the VBO to the fragment shader
    v_texCoord = vertexUV;

    // Set the vertex position to the value from the VBO, not applying anything clever here :o)
    gl_Position = vec4(vertexPosition, 1.0);
}
