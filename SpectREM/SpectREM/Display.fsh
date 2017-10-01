#version 330

// Texture coordinates passed in from the vertex shader
in vec2 v_texCoord;

// Fragment colour final output
out vec4 out_fragColor;

// Texture to be processed
uniform sampler2D displayTexture;

// Uniforms linked to different screen settings
uniform float borderSize;


///////////////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////////////
void main()
{
    // Calculate the non-bright colour to be used
    float normalColor = 208.0 / 255.0;

    vec3 CLUT[16] = vec3[](
                           // Non-bright colours
                           vec3( 0.0, 0.0, 0.0 ),
                           vec3( 0.0, 0.0, normalColor ),
                           vec3( normalColor, 0.0, 0.0 ),
                           vec3( normalColor, 0.0, normalColor),
                           vec3( 0.0, normalColor, 0.0),
                           vec3( 0.0, normalColor, normalColor ),
                           vec3( normalColor, normalColor, 0.0 ),
                           vec3( normalColor, normalColor, normalColor ),
                           
                           // Bright colours
                           vec3( 0.0, 0.0, 0.0 ),
                           vec3( 0.5, 0.0, 1.0 ),
                           vec3( 1.0, 0.0, 0.0 ),
                           vec3( 1.0, 0.0, 1.0),
                           vec3( 0.0, 1.0, 0.0),
                           vec3( 0.0, 1.0, 1.0 ),
                           vec3( 1.0, 1.0, 0.0 ),
                           vec3( 1.0, 1.0, 1.0 )
                           );
    
    // Variables to be used for calculating the size of the border to be drawn
    const float w = 320;
    const float h = 256;
    float border = 32 - borderSize;
    float new_w = w - (border * 2);
    float new_h = h - (border * 2);
    
    // Flip the Y coord otherwise the image renders upside down
    vec2 texCoord = v_texCoord * vec2(1.0, -1.0);
    
    float u = ((texCoord.x * new_w) + border);
    float v = ((texCoord.y * new_h) - border);
    vec2 vUv = vec2(u, v);
    texCoord = (vUv / vec2(w, h));

    // Get the colour to be used from the texture passed in
    float c = texture( displayTexture, texCoord).r * 255;

    // Grab the actual colour from the lookup table
    out_fragColor = vec4(CLUT[ int(c) ], 1.0);
}
