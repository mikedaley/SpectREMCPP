//
//  MetalView.m
//  SpectREM
//
//  Created by Mike on 20/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

@import simd;
@import MetalKit;

#import "MetalView.h"
#import "ShaderTypes.h"

// Constants for the colour lookup table
const GLfloat normalColor = 189.0 / 255.0;
const GLfloat brightColor = 1.0;
const Color CLUT[] = {
    
    // Non-bright colours
    { 0.0,          0.0,            0.0,            1.0 },
    { 0.0,          0.0,            normalColor,    1.0 },
    { normalColor,  0.0,            0.0,            1.0 },
    { normalColor,  0.0,            normalColor,    1.0 },
    { 0.0,          normalColor,    0.0,            1.0 },
    { 0.0,          normalColor,    normalColor,    1.0 },
    { normalColor,  normalColor,    0.0,            1.0 },
    { normalColor,  normalColor,    normalColor,    1.0 },
    
    // Bright colours
    { 0.0,          0.0,            0.0,            1.0 },
    { 0.0,          0.0,            brightColor,    1.0 },
    { brightColor,  0.0,            0.0,            1.0 },
    { brightColor,  0.0,            brightColor,    1.0 },
    { 0.0,          brightColor,    0.0,            1.0 },
    { 0.0,          brightColor,    brightColor,    1.0 },
    { brightColor,  brightColor,    0.0,            1.0 },
    { brightColor,  brightColor,    brightColor,    1.0 },
};

static const Vertex quadVertices[] =
{
    // Pixel positions, Texture coordinates flipped on Y axis
    { {  1.0,  -1.0 },  { 1.f, 1.f } },
    { { -1.0,  -1.0 },  { 0.f, 1.f } },
    { { -1.0,   1.0 },  { 0.f, 0.f } },
    
    { {  1.0,  -1.0 },  { 1.f, 1.f } },
    { { -1.0,   1.0 },  { 0.f, 0.f } },
    { {  1.0,   1.0 },  { 1.f, 0.f } },
};

@implementation MetalView
{
    dispatch_semaphore_t _inFlightSemaphore;
    
    // The device (aka GPU) we're using to render
    id<MTLDevice> _device;
    
    // Our render pipeline composed of our vertex and fragment shaders in the .metal shader file
    id<MTLRenderPipelineState> _pipelineState;
    
    // The command Queue from which we'll obtain command buffers
    id<MTLCommandQueue> _commandQueue;
    
    // The Metal texture object
    id<MTLTexture> _displayTexture;
    id<MTLTexture> _clutTexture;

    // The Metal buffer in which we store our vertex data
    id<MTLBuffer> _vertices;
    
    // The number of vertices in our vertex buffer
    NSUInteger _numVertices;
    
    // The current size of our view so we can use this in our render pipeline
    vector_uint2 _viewportSize;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView
{
    self = [super init];
    if (self)
    {
        _device = mtkView.device;
        
        _inFlightSemaphore = dispatch_semaphore_create(3);
        MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
        
        // Emulator display texture
        textureDescriptor.pixelFormat = MTLPixelFormatR8Unorm;
        textureDescriptor.textureType = MTLTextureType2D;
        textureDescriptor.width = 320;
        textureDescriptor.height = 256;
        
        _displayTexture = [_device newTextureWithDescriptor:textureDescriptor];
        
        // Colour lookup texture
        textureDescriptor.pixelFormat = MTLPixelFormatRGBA32Float;
        textureDescriptor.textureType = MTLTextureType1D;
        textureDescriptor.width = 16;
        textureDescriptor.height = 1;
    
        _clutTexture = [_device newTextureWithDescriptor:textureDescriptor];
    
        MTLRegion region = {
            {0, 0, 0}, // Origin
            {16, 1, 1} // Size
        };
        
        [_clutTexture replaceRegion:region mipmapLevel:0 withBytes:&CLUT bytesPerRow:16 * sizeof(Color)];
        

    
        // Create vertex buffer and init with the quadVertices array
        _vertices = [_device newBufferWithBytes:quadVertices
                                         length:sizeof(quadVertices)
                                        options:MTLResourceStorageModeShared];
        
        // Calculate the number of vertices
        _numVertices = sizeof(quadVertices) / sizeof(Vertex);
        
        // Create the render pipeline
        
        // Load all the shaders with a .metal file extension
        id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
    
        id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"samplingShader"];
        
        // Setup the descriptor for creating a pipeline state object
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"Texture Pipeline";
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
        
        NSError *error = NULL;
        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
        
        if (!_pipelineState)
        {
            NSLog(@"Failed to create pipeline state, error %@", error);
        }
        
        // Create the command queue
        _commandQueue = [_device newCommandQueue];
        
    }
    return self;
}

- (void)updateTextureData:(void *)displayBuffer
{
    MTLRegion region = {
        {0, 0, 0}, // Origin
        {320, 256, 1} // Size
    };
    
    [_displayTexture replaceRegion:region mipmapLevel:0 withBytes:displayBuffer bytesPerRow:320];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    
    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);
    
    // Create a new command buffer for each render pass to the current drawable
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MyCommand";
    
    __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
     {
         dispatch_semaphore_signal(block_sema);
     }];
    
    // Obtain a renderPassDescriptor generated from the view's drawable textures
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
    
    if(renderPassDescriptor != nil)
    {
        // Create a render command encoder so we can render into something
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"MyRenderEncoder";
        
        // Set the region of the drawable to which we'll draw.
        [renderEncoder setViewport:(MTLViewport){0.0, 0.0, _viewportSize.x, _viewportSize.y, -1.0, 1.0 }];
        
        [renderEncoder setRenderPipelineState:_pipelineState];
        
        [renderEncoder setVertexBuffer:_vertices
                                offset:0
                               atIndex:VertexInputIndexVertices];
        
        [renderEncoder setVertexBytes:&_viewportSize
                               length:sizeof(_viewportSize)
                              atIndex:VertexInputIndexViewportSize];
        
        // Setup the texture containing the emulator output
        [renderEncoder setFragmentTexture:_displayTexture
                                  atIndex:TextureIndexBaseColor];

        // Setup the texture with the colour lookup
        [renderEncoder setFragmentTexture:_clutTexture
                                  atIndex:TextureCLUTColor];

        // Draw the vertices of our triangles
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                          vertexStart:0
                          vertexCount:_numVertices];
        
        [renderEncoder endEncoding];
        
        // Schedule a present once the framebuffer is complete using the current drawable
        [commandBuffer presentDrawable:view.currentDrawable];
    }
    
    // Finalize rendering here & push the command buffer to the GPU
    [commandBuffer commit];
    
}

@end
