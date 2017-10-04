#version 330

// Texture coordinates passed in from the vertex shader
in vec2 v_texCoord;

// Fragment colour final output
out vec4 out_fragColor;

// Texture to be processed
uniform sampler2D displayTexture;

// Uniforms linked to different screen settings
uniform float u_borderSize;
uniform float u_contrast;
uniform float u_saturation;
uniform float u_brightness;

///////////////////////////////////////////////////////////////////////////////////////
// colorCorrection used to adjust the saturation, constrast and brightness of the image
///////////////////////////////////////////////////////////////////////////////////////
vec3 colorCorrection(vec3 color, float saturation, float contrast, float brightness)
{
    const vec3 meanLuminosity = vec3(0.5, 0.5, 0.5);
    const vec3 rgb2greyCoeff = vec3(0.2126, 0.7152, 0.0722);    // Updated greyscal coefficients for sRGB and modern TVs
    
    vec3 brightened = color * brightness;
    float intensity = dot(brightened, rgb2greyCoeff);
    vec3 saturated = mix(vec3(intensity), brightened, saturation);
    vec3 contrasted = mix(meanLuminosity, saturated, contrast);
    
    return contrasted;
}

///////////////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////////////
void main()
{
    // Calculate the non-bright colour to be used
    const float normalColor = 208.0 / 255.0;
    const float brightColor = 1.0;
    
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
                           vec3( 0.0, 0.0, brightColor ),
                           vec3( brightColor, 0.0, 0.0 ),
                           vec3( brightColor, 0.0, brightColor),
                           vec3( 0.0, brightColor, 0.0),
                           vec3( 0.0, brightColor, brightColor ),
                           vec3( brightColor, brightColor, 0.0 ),
                           vec3( brightColor, brightColor, brightColor )
                           );
    
    // Variables to be used for calculating the size of the border to be drawn
    const float w = 320;
    const float h = 256;
    float border = 32 - u_borderSize;
    float new_w = w - (border * 2);
    float new_h = h - (border * 2);
    
    // Flip the Y coord otherwise the image renders upside down
    vec2 texCoord = v_texCoord * vec2(1.0, -1.0);
    
    // Update the UV coordinates based on the size of the border
    float u = ((texCoord.x * new_w) + border);
    float v = ((texCoord.y * new_h) - border);
    vec2 vUv = vec2(u, v);
    texCoord = (vUv / vec2(w, h));
    
    // Get the colour to be used from the texture passed in
    float c = texture( displayTexture, texCoord).r * 255;
    
    // Grab the actual colour from the lookup table
    vec3 color = CLUT[ int(c) ];
    
    // Adjust colour based on contrast, saturation and brightness
    color = colorCorrection(color, u_saturation, u_contrast, u_brightness);
    
    out_fragColor = vec4(color, 1.0);
}

