//
//  ShaderTypes.h
//  SpectREM
//
//  Created by Mike on 20/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <simd/simd.h>

// Buffer index values shared between shader and C code to ensure Metal shader buffer inputs match Metal API buffer set calls
typedef enum VertexInputIndex
{
    VertexInputIndexVertices = 0,
    VertexInputIndexViewportSize = 1,
} VertexInputIndex;

// Texture index values shared between shader and C code to ensure Metal shader buffer inputs match Metal API texture set calls
typedef enum TextureIndex
{
    TextureIndexBaseColor = 0,
    TextureCLUTColor = 1,
    TextureUniforms = 2,
} TextureIndex;

// This structure defines the layout of each vertex in the array of vertices set as an input to the Metal
// vertex shader. Since this header is shared between the .metal shader and C code, we can be sure that the
// layout of the vertex array in the code matches the layout that the shader expects
typedef struct
{
    // Position in pixel space
    vector_float2 position;
    
    // 2D texture coord
    vector_float2 textureCoordinate;
} Vertex;

typedef struct
{
    float r;
    float g;
    float b;
    float a;
} Color;

typedef struct
{
    float displayCurvature;
    float displayBorderSize;
    float displayPixelFilterValue;
} Uniforms;

#endif /* ShaderTypes_h */
