#version 330

// Texture coordinates passed in from the vertex shader
in vec2 v_texCoord;

// Fragment colour final output
out vec4 out_fragColor;

// Texture to be processed
uniform sampler2D s_displayTexture;

// Uniforms linked to different screen settings
uniform float u_borderSize;
uniform float u_contrast;
uniform float u_saturation;
uniform float u_brightness;
uniform float u_scanlineSize;
uniform float u_scanlines;
uniform float u_screenCurve;
uniform float u_pixelFilterValue;
uniform float u_rgbOffset;

uniform float u_time;

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
// Distortion used to give the screen a curved effect
///////////////////////////////////////////////////////////////////////////////////////
vec2 radialDistortion(vec2 pos, float distortion)
{
    vec2 cc = pos - vec2(0.5, 0.5);
    float dist = dot(cc, cc) * distortion;
    return (pos + cc * (0.5 + dist) * dist);
}

///////////////////////////////////////////////////////////////////////////////////////
// Split the red, green and blue channels of the texture passed in
///////////////////////////////////////////////////////////////////////////////////////
vec3 channelSplit(sampler2D tex, vec2 coord, float spread){
    vec3 frag;
    frag.r = texture(tex, vec2(coord.x - spread, coord.y)).r;
    frag.g = texture(tex, vec2(coord.x, coord.y)).g;
    frag.b = texture(tex, vec2(coord.x + spread, coord.y)).b;
    return frag;
}

///////////////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////////////
void main()
{
    // Variables to be used for calculating the size of the border to be drawn
    const float w = 320;
    const float h = 256;
    float border = 32 - u_borderSize;
    float new_w = w - (border * 2);
    float new_h = h - (border * 2);
    vec4 color;
    
    // Apply screen curve
    vec2 texCoord = radialDistortion(v_texCoord, u_screenCurve);
    vec2 scanTexCoord = texCoord;
    
    if (texCoord.x < 0 || texCoord.y < 0 || texCoord.x > 1 || texCoord.y > 1)
    {
        color = vec4(0.1, 0.1, 0.1, 1.0);
    }
    else
    {
//        // Flip the Y coord otherwise the image renders upside down
        texCoord = texCoord * vec2(1.0, -1.0);
        
        // Update the UV coordinates based on the size of the border
        float u = ((texCoord.x * new_w) + border);
        float v = ((texCoord.y * new_h) - border);
        
        // Apply pixel filtering
        vec2 vUv = vec2(u, v);
        vec2 alpha = vec2(u_pixelFilterValue); // 0.5 = Linear, 0.0 = Nearest
        vec2 x = fract(vUv);
        vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
        texCoord = (floor(vUv) + x_) / vec2(w, h);
        
        // Apply RGB shift if necessary, otherwise just take the colour from the texture as is
        if (u_rgbOffset > 0)
        {
            color.rgb = channelSplit( s_displayTexture, texCoord, u_rgbOffset);
        }
        else
        {
            // Grab the color from the texture
            color = texture( s_displayTexture, texCoord );
        }
        
        // Adjust colour based on contrast, saturation and brightness
        color.rgb = colorCorrection(color.rgb, u_saturation, u_contrast, u_brightness);
        
        // Add scanlines
        float scanline = sin(scanTexCoord.y * u_scanlineSize) * 0.09 * u_scanlines;
        color.rgb -= scanline;
    }
    
    // Output the final colour
    out_fragColor = vec4(color.rgb, 1.0);
}











//const float e = 2.7182818284590452353602874713527;
//
//vec4 noise(vec2 texCoord)
//{
//    float G = e + (iTime * 0.1);
//    vec2 r = (G * sin(G * texCoord.xy));
//    return vec4(fract(r.x * r.y * (1.0 + texCoord.x)));
//}
//
//
//void mainImage(out vec4 o, vec2 texCoord)
//{
//    o = noise(texCoord + 1000.);
//}

//vec3 scanline(vec2 coord, vec3 screen){
//    const float scale = .66;
//    const float amt = 0.02;// intensity of effect
//    const float spd = 0.5;//speed of scrolling rows transposed per second
//
//    screen.rgb += sin((coord.y / scale - (u_time * spd * 6.28))) * amt;
//    return screen;
//}

