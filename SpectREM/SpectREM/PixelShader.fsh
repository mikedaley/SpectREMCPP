//
//  Shader.fsh
//  SpectREM
//
//  Created by Michael Daley on 31/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x * 34.0) + 1.0) * x);
}

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    
    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    
    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                     + i.x + vec3(0.0, i1.x, 1.0 ));
    
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    
    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
    
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    
    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0 * a0 + h * h );
    
    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

// Distortion used to provide a curved screen effect
vec2 radialDistortion(vec2 pos, float distortion)
{
    vec2 cc = pos - vec2(0.5, 0.5);
    float dist = dot(cc, cc) * distortion;
    return (pos + cc * (0.5 + dist) * dist);
}

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

vec3 vegnetteColor(vec3 color, vec2 coord, float vig_x, float vig_y)
{
    float dist = distance(coord, vec2(0.5, 0.5));
    return vec3(smoothstep(vig_x, vig_y, dist));
}

void main()
{
    const float w = 320;
    const float h = 256;
    const float border = 32 - u_displayBorderSize;
    const float new_w = w - (border * 2);
    const float new_h = h - (border * 2);
    
    vec3 color;
    float yOffset = 0;
    float xOffset = 0;
    float fuzzOffset = 0;
    float largeFuzzOffset = 0;
    float staticVal = 0;
    
    // Apply screen curve
    v_tex_coord = radialDistortion(v_tex_coord, u_displayCurvature);
    
    // Anything outside of the screens texture coordinates just draw a fixed colour
    if (v_tex_coord.x < 0 || v_tex_coord.y < 0 || v_tex_coord.x > 1 || v_tex_coord.y > 1)
    {
        color = vec3(0.1, 0.1, 0.1);
    }
    else
    {
        // Get the u coordinate based on the size of the new texture - this is in pixels
        float u = ((v_tex_coord.x * new_w) + border);
        float v = ((v_tex_coord.y * new_h) + border);
        
        // Apply parametric nearest filtering
        vec2 vUv = vec2(u, v);
        vec2 alpha = vec2(u_displayFilterValue); // 0.5 = Linear, 0.0 = Nearest
        vec2 x = fract(vUv);
        vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
        
        vec2 texCoord = (floor(vUv) + x_) / vec2(w, h);
        
        if (u_displayHorizontalSync > 0.0)
        {
            fuzzOffset = snoise(vec2(u_time * 15.0, texCoord.y * 80.0)) * 0.003;
            largeFuzzOffset = snoise(vec2(u_time * 1.0, texCoord.y * 25.0)) * 0.004;
        }
        
        float y = mod(texCoord.y + yOffset, 1.0);
        xOffset = (fuzzOffset + largeFuzzOffset) * u_displayHorizontalSync;

        float red   =   texture2D( u_texture, vec2( texCoord.x + xOffset - 0.01 * u_displayRGBOffset, y ) ).r + staticVal;
        float green =   texture2D( u_texture, vec2( texCoord.x + xOffset, y)).g + staticVal;
        float blue  =   texture2D( u_texture, vec2( texCoord.x + xOffset + 0.01 * u_displayRGBOffset, y ) ).b + staticVal;
        
        color = vec3(red,green,blue);
        
        color = colorCorrection(color, u_displaySaturation, u_displayContrast, u_displayBrightness);
        
        float scanline = sin(v_tex_coord.y * 880) * 0.04 * u_displayScanLines;
        color -= scanline;
        
        vec3 vignette = vegnetteColor(color, texCoord, u_displayVignetteX, u_displayVignetteY);
        
        if (u_displayShowVignette == 1.0)
        {
            color *= vignette;
        }
    }

    gl_FragColor = vec4(color, 1.0);
}
