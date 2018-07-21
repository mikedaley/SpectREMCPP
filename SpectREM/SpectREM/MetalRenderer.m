//
//  MetalView.m
//  SpectREM
//
//  Created by Mike on 20/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

@import simd;
@import MetalKit;

#import "MetalRenderer.h"
#import "ShaderTypes.h"
#import "Defaults.h"

static const NSUInteger MaxBuffersInFlight = 3;

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

@implementation MetalRenderer

{
    dispatch_semaphore_t _inFlightSemaphore;
    
    // The device (aka GPU) we're using to render
    id<MTLDevice> _device;
    
    id<MTLRenderPipelineState> _clutPipelineState;
    id<MTLRenderPipelineState> _effectsPipelineState;

    MTLRenderPassDescriptor *_clutPassDescriptor;
    MTLRenderPassDescriptor *_effectsPassDescriptor;
    
    id<MTLBuffer> _vertexBuffers[MaxBuffersInFlight];
    NSUInteger _currentBuffer;
    
    // The command Queue from which we'll obtain command buffers
    id<MTLCommandQueue> _commandQueue;
    
    // The Metal texture object
    id<MTLTexture> _displayTexture;
    id<MTLTexture> _clutTexture;
    id<MTLTexture> _effectsTexture;

    // The Metal buffer in which we store our vertex data
    id<MTLBuffer> _vertices;
    
    // The number of vertices in our vertex buffer
    NSUInteger _numVertices;
    
    // The current size of our view so we can use this in our render pipeline
    vector_uint2 _viewportSize;
    
    MTKView *_view;
    Uniforms _uniforms;
    Defaults *_defaults;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView
{
    self = [super init];
    if (self)
    {
        _defaults = [Defaults defaults];
        
        mtkView.paused = YES;
        _device = mtkView.device;
        
        _view = mtkView;

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

        // Load lookup texture with standard ZX Spectrum colour palette
        MTLRegion region = {
            {0, 0, 0}, // Origin
            {16, 1, 1} // Size
        };
        [_clutTexture replaceRegion:region mipmapLevel:0 withBytes:&CLUT bytesPerRow:16 * sizeof(Color)];
        
        // Texture used to apply final output effects
        textureDescriptor.pixelFormat = MTLPixelFormatRGBA32Float;
        textureDescriptor.textureType = MTLTextureType2D;
        textureDescriptor.width = 320;
        textureDescriptor.height = 256;
        textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        _effectsTexture = [_device newTextureWithDescriptor:textureDescriptor];
    
        // Create a number of vertex buffers so that we can be working on different buffers at the same time
        for(NSUInteger bufferIndex = 0; bufferIndex < MaxBuffersInFlight; bufferIndex++)
        {
            _vertexBuffers[bufferIndex] = [_device newBufferWithBytes:quadVertices
                                                               length:sizeof(quadVertices)
                                                              options:MTLResourceStorageModeShared];
        }

        // Calculate the number of vertices
        _numVertices = sizeof(quadVertices) / sizeof(Vertex);
        
        // Create the render pipeline
        
        // Load all the shaders with a .metal file extension
        id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
    
        id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
        id<MTLFunction> clutFragmentFunction = [defaultLibrary newFunctionWithName:@"clutShader"];
        id<MTLFunction> effectsFragmentFunction = [defaultLibrary newFunctionWithName:@"effectsShader"];

        // Setup the descriptor for creating a pipeline state object
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"CLUT Pipeline";
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = clutFragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA32Float;
        
        NSError *error = NULL;
        _clutPipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
        
        if (!_clutPipelineState)
        {
            NSLog(@"Failed to create pipeline state, error %@", error);
        }
        
        // Setup the descriptor for creating a pipeline state object
        pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"Effects Pipeline";
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = effectsFragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
        
        error = NULL;
        _effectsPipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
        
        if (!_clutPipelineState)
        {
            NSLog(@"Failed to create pipeline state, error %@", error);
        }
        
        // Create the command queue
        _commandQueue = [_device newCommandQueue];
        
        _clutPassDescriptor = [MTLRenderPassDescriptor new];
        _clutPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        _clutPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        _clutPassDescriptor.colorAttachments[0].texture = _effectsTexture;
        
        _effectsPassDescriptor = [MTLRenderPassDescriptor new];
        _effectsPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        
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

    dispatch_async(dispatch_get_main_queue(), ^{
        [_view draw];
    });
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    
    // Wait to ensure only MaxBuffersInFlight number of frames are getting proccessed
    //   by any stage in the Metal pipeline (App, Metal, Drivers, GPU, etc)
    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);
    
    // Iterate through our Metal buffers, and cycle back to the first when we've written to MaxBuffersInFlight
    _currentBuffer = (_currentBuffer + 1) % MaxBuffersInFlight;

    // Create a new command buffer for each render pass to the current drawable
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"RENDER COMMANDS";
    
    // Add completion handler which signals _inFlightSemaphore when Metal and the GPU has fully
    //   finished processing the commands we're encoding this frame.  This indicates when the
    //   dynamic buffers filled with our vertices, that we're writing to this frame, will no longer
    //   be needed by Metal and the GPU, meaning we can overwrite the buffer contents without
    //   corrupting the rendering.
    __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
     {
         dispatch_semaphore_signal(block_sema);
     }];
    
    if (_clutPassDescriptor != nil)
    {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:_clutPassDescriptor];
        renderEncoder.label = @"CLUT RENDERER";
        
        [renderEncoder setViewport:(MTLViewport){0.0, 0.0, 320, 256, -1.0, 1.0 }];
        

        [renderEncoder setRenderPipelineState:_clutPipelineState];

        
        [renderEncoder setVertexBuffer:_vertexBuffers[_currentBuffer]
                                offset:0
                               atIndex:VertexInputIndexVertices];
        
        [renderEncoder setVertexBytes:&_viewportSize
                               length:sizeof(_viewportSize)
                              atIndex:VertexInputIndexViewportSize];
        
        [renderEncoder setFragmentTexture:_displayTexture
                                  atIndex:TextureIndexPackedDisplay];
        
        [renderEncoder setFragmentTexture:_clutTexture
                                  atIndex:TextureIndexCLUT];
        
        // Draw the vertices of our triangles
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                          vertexStart:0
                          vertexCount:_numVertices];
        
        [renderEncoder endEncoding];
    }
    
    if(_effectsPassDescriptor != nil)
    {
        // Create a render command encoder so we can render into something

        _effectsPassDescriptor.colorAttachments[0].texture = view.currentDrawable.texture;

        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:_effectsPassDescriptor];
        renderEncoder.label = @"EFFECTS RENDERER";
        
        // Set the region of the drawable to which we'll draw.
        [renderEncoder setViewport:(MTLViewport){0.0, 0.0, _viewportSize.x, _viewportSize.y, -1.0, 1.0 }];
        
        [renderEncoder setRenderPipelineState:_effectsPipelineState];
        
        [renderEncoder setVertexBuffer:_vertexBuffers[_currentBuffer]
                                offset:0
                               atIndex:VertexInputIndexVertices];
        
        [renderEncoder setVertexBytes:&_viewportSize
                               length:sizeof(_viewportSize)
                              atIndex:VertexInputIndexViewportSize];
        
        // Setup the texture containing the emulator output
        [renderEncoder setFragmentTexture:_effectsTexture
                                  atIndex:0];
        
        _uniforms.displayCurvature = _defaults.displayCurvature;
        _uniforms.displayBorderSize = _defaults.displayBorderSize;
        _uniforms.displayPixelFilterValue = _defaults.displayPixelFilterValue;
        
        [renderEncoder setFragmentBytes:&_uniforms
                                 length:sizeof(_uniforms)
                                atIndex:0];

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
