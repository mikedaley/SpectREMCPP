//
//  Display.metal
//  SpectREM
//
//  Created by Mike on 20/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>

#import "ShaderTypes.h"

// Vertex shader outputs and per-fragment inputs. Includes clip-space position and vertex outputs
// interpolated by rasterizer and fed to each fragment generated by clip-space primitives.
typedef struct
{
    float4 clipSpacePosition [[position]];
    float2 textureCoordinate;
    
} RasterizerData;

///////////////////////////////////////////////////////////////////////////////////////
// Distortion used to give the screen a curved effect
///////////////////////////////////////////////////////////////////////////////////////
vector_float2 radialDistortion(vector_float2 pos, float distortion)
{
    vector_float2 cc = pos - vector_float2(0.5, 0.5);
    float dist = metal::dot(cc, cc) * distortion;
    return (pos + cc * (0.5 + dist) * dist);
}

vector_float3 scanline(float2 texCoord, float3 fragColor, float time, float amount, float size) {
    const float scale = 0.0008;
    const float amt = amount;// intensity of effect
    const float spd = -0.03;//speed of scrolling rows transposed per second
    fragColor.rgb -= metal::sin( (texCoord.y / scale - (time * spd * 6.28) ) ) * amt;
    return fragColor;
}

///////////////////////////////////////////////////////////////////////////////////////
// colorCorrection used to adjust the saturation, constrast and brightness of the image
///////////////////////////////////////////////////////////////////////////////////////
vector_float3 colorCorrection(vector_float3 color, float saturation, float contrast, float brightness)
{
    const vector_float3 meanLuminosity = vector_float3(0.5, 0.5, 0.5);
    const vector_float3 rgb2greyCoeff = vector_float3(0.2126, 0.7152, 0.0722);    // Updated greyscale coefficients for sRGB and modern TVs
    
    vector_float3 brightened = color * brightness;
    float intensity = metal::dot(brightened, rgb2greyCoeff);
    vector_float3 saturated = metal::mix(vector_float3(intensity), brightened, saturation);

    return metal::mix(meanLuminosity, saturated, contrast);
}

///////////////////////////////////////////////////////////////////////////////////////
// Split the red, green and blue channels of the texture passed in
///////////////////////////////////////////////////////////////////////////////////////
vector_float4 channelSplit(metal::texture2d<float>image, metal::sampler tex, vector_float2 coord, float spread){
    vector_float4 frag;
    frag.r = image.sample(tex, vector_float2(coord.x - spread, coord.y)).r;
    frag.g = image.sample(tex, vector_float2(coord.x, coord.y)).g;
    frag.b = image.sample(tex, vector_float2(coord.x + spread, coord.y)).b;
    frag.a = 1.0;
    return frag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 VERTEX SHADER
 Vertex shader used to pass the vertices through to the fragment shader. Not much going on in here :o)
 **/
vertex RasterizerData vertexShader(uint vertexID [[ vertex_id ]],
                                   constant Vertex *vertexArray [[ buffer(VertexInputIndexVertices) ]],
                                   constant vector_uint2 *viewportSizePointer [[ buffer(VertexInputIndexViewportSize) ]])
{
    float2 pixelSpacePosition = vertexArray[vertexID].position.xy;
    RasterizerData out;
    out.clipSpacePosition.xy = pixelSpacePosition;
    out.clipSpacePosition.z = 0.0;
    out.clipSpacePosition.w = 1.0;
    out.textureCoordinate = vertexArray[vertexID].textureCoordinate;
    
    return out;
}

/**
 COLOUR LOOKUP FRAGMENT SHADER
 Used to lookup the read colours to use from the packed image data provided by the emaultor. The emulator
 output provides a single byte per pixel and the value of that byte is used to lookup the actual colour from the colour
 lookup texture
 **/
fragment vector_float4 clutShader( RasterizerData in [[stage_in]],
                                  metal::texture2d<half> colorTexture [[ texture(TextureIndexPackedDisplay) ]],
                                  metal::texture1d<float> clutTexture [[ texture(TextureIndexCLUT) ]])
{
    constexpr metal::sampler textureSampler (metal::mag_filter::nearest,
                                      metal::min_filter::nearest);

    const float paletteColors = 16.0;
    const float clutUVAdjust = 1.0 / paletteColors;
    const int colorSample = colorTexture.sample(textureSampler, in.textureCoordinate)[0] * 256;
    return clutTexture.sample(textureSampler, colorSample * clutUVAdjust);
}

/**
 EFFECTS FRAGMENT SHADER
 Used to apply effects to the output from the colour CLUT shader. The output from this shader is what is
 then displayed on screen
 **/
fragment vector_float4 effectsShader(RasterizerData in [[stage_in]],
                                     metal::texture2d<float> colorTexture [[ texture(0) ]],
                                     constant Uniforms & uniforms [[buffer(0) ]])
{
    constexpr metal::sampler textureSampler (metal::mag_filter::linear,
                                             metal::min_filter::linear);
    
    float max = metal::pow(uniforms.displayVignetteX, uniforms.displayVignetteY);
    const float w = 32 + 256 + 32;
    const float h = 32 + 192 + 32;
    float border = 32 - uniforms.displayBorderSize;
    float new_w = w - (border * 2);
    float new_h = h - (border * 2);
    vector_float4 fragColor;

    vector_float2 texCoord = radialDistortion(in.textureCoordinate, uniforms.displayCurvature);
    vector_float2 scanTexCoord = texCoord;

    // Anything outside the texture should be black, otherwise sample the texel in the texture
    if (texCoord.x < 0 || texCoord.y < 0 || texCoord.x > 1 || texCoord.y > 1)
    {
        fragColor = vector_float4(0.0, 0.0, 0.0, 1);
    }
    else
    {
        // Update the UV coordinates based on the size of the border
        float u = (texCoord.x * new_w) + border;
        float v = ((texCoord.y * new_h) + border);

        // Apply pixel filtering
        vector_float2 vUv = vector_float2(u, v);
        float alpha = uniforms.displayPixelFilterValue; // 0.5 = Linear, 0.0 = Nearest
        vector_float2 x = metal::fract(vUv);
        vector_float2 x_ = metal::clamp(0.5 / alpha * x, 0.0, 0.5) + metal::clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
        texCoord = (metal::floor(vUv) + x_) / vector_float2(w, h);

        // Apply RGB shift if necessary, otherwise just take the colour from the texture as is
        if (uniforms.displayRGBOffset > 0)
        {
            fragColor = channelSplit( colorTexture, textureSampler, texCoord, uniforms.displayRGBOffset);
        }
        else
        {
            // Grab the color from the texture
            fragColor = colorTexture.sample( textureSampler, texCoord );
        }
        
        fragColor.rgb = colorCorrection(fragColor.rgb, uniforms.displaySaturation, uniforms.displayContrast, uniforms.displayBrightness);
        
        // Add scanlines
//        float scanline = sin(scanTexCoord.y * uniforms.displayScanlineSize) * 0.09 * uniforms.displayScanlines;
//        fragColor.rgb -= scanline;
        
//        fragColor.rgb = scanline(scanTexCoord.xy, fragColor.rgb, uniforms.time, uniforms.displayScanlines, uniforms.displayScanlineSize);
        fragColor.rgb = scanline(scanTexCoord.xy, fragColor.rgb, 0, uniforms.displayScanlines, uniforms.displayScanlineSize);

        // Add the vignette which adds a shadow and curving to the corners of the screen. Do this after applying the scan lines so
        // they are faded out into the shadow as well
        if (uniforms.displayShowVignette)
        {
            float vignette = scanTexCoord.x * scanTexCoord.y * (1.0 - scanTexCoord.x) * (1.0 - scanTexCoord.y);
            fragColor.rgb *= metal::smoothstep(0.0, max, vignette);
        }
    }
    // We return the color of the texture
    return fragColor;
}
