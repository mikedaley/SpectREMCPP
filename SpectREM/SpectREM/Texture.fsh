#version 330

// Texture coordinates passed in from the vertex shader
in vec2 v_texCoord;

// Fragment colour final output
layout(location = 0) out vec4 out_fragColor;

// Texture to be processed
uniform sampler2D displayTexture;
uniform sampler1D clutTexture;

const float clutUVAdjust = 1.0 / 16.0;

///////////////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////////////
void main()
{
    // Get the colour to be used from the texture passed in
    float c = texture( displayTexture, v_texCoord ).r * 255;
    
    // Grab the actual colour from the lookup table
    vec4 color = texture( clutTexture, c * clutUVAdjust );
    
    // Output the final colour
    out_fragColor = color;
}

