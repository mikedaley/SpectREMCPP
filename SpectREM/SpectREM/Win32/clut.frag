#version 330

// Texture coordinates passed in from the vertex shader
in vec2 v_texCoord;

// Fragment colour final output
out vec4 out_fragColor;

// Texture to be processed
uniform sampler2D s_displayTexture;
uniform sampler1D s_clutTexture;

// CLUT is 16 pixels wide, so work out the UV step per colour
const float clutUVAdjust = 1.0 / 16.0;

///////////////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////////////
void main()
{
    // Get the colour to be used from the texture passed in
    float c = texture( s_displayTexture, v_texCoord ).r * 256;

    // Grab the actual colour from the lookup table
    vec4 color = texture( s_clutTexture, c * clutUVAdjust );

    // Output the final colour
    out_fragColor = color;
}
