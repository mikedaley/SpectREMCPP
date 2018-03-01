//
//  OpenGLView.m
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <CoreMedia/CoreMedia.h>
#import "OpenGLView.h"
#import "EmulationViewController.h"
#import "Defaults.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#pragma mark - Constants

/**
     3-----2 <--  1
     |\    |
     |  \  | <--  0
     |    \|
     0-----1 <-- -1

     ^  ^  ^
     |  |  |
    -1  0  1
 **/
const GLfloat quad[] = {
    //X      Y      Z        U      V
    -1.0f,  -1.0f,  0.0f,    0.0f,  0.0f, // 0
     1.0f,  -1.0f,  0.0f,    1.0f,  0.0f, // 1
     1.0f,   1.0f,  0.0f,    1.0f,  1.0f, // 2
    -1.0f,   1.0f,  0.0f,    0.0f,  1.0f  // 3
};

typedef struct
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
} Color;

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
    { brightColor,  brightColor,    brightColor,    1.0 }
};

const GLuint textureUnit0 = 0;
const GLuint textureUnit1 = 1;
const GLuint textureUnit2 = 2;

const GLuint border = 32;
const GLuint screenWidth = border + 256 + border;
const GLuint screenHeight = border + 192 + border;

char const * cS_DISPLAY_TEXTURE =       "s_displayTexture";
char const * cS_CLUT_TEXTURE =          "s_clutTexture";
char const *cS_REFLECTION_TEXTURE =     "s_reflectionTexture";
char const * cU_BORDER_SIZE =           "u_borderSize";
char const * cU_CONTRAST =              "u_contrast";
char const * cU_SATURATION =            "u_saturation";
char const * cU_BRIGHTNESS =            "u_brightness";
char const * cU_PIXEL_FILTER_VALUE =    "u_pixelFilterValue";
char const * cU_SCAN_LINE_SIZE =        "u_scanlineSize";
char const * cU_SCAN_LINES =            "u_scanlines";
char const * cU_SCREEN_CURVE =          "u_screenCurve";
char const * cU_RGB_OFFSET =            "u_rgbOffset";
char const * cU_SHOW_VIGNETTE =         "u_showVignette";
char const * cU_VIGNETTE_X =            "u_vignetteX";
char const * cU_VIGNETTE_Y =            "u_vignetteY";
char const * cU_SHOW_REFLECTION =       "u_showReflection";
char const * cU_TIME =                  "u_time";
char const * cU_SCREEN_SIZE =           "u_screenSize";

#pragma mark - Private Ivars

@interface OpenGLView ()
{
    NSTrackingArea *trackingArea;
    NSWindowController *windowController;
    CGLContextObj contextObj;
    NSOpenGLContext *context;
    
    float          viewWidth;
    float          viewHeight;
    
    GLuint          vertexBuffer;
    GLuint          vertexArray;

    // Shader names
    GLint           clutShader;
    GLint           displayShader;
    
    // CLUT shader uniforms/samplers
    GLuint          clutFrameBuffer;
    GLuint          clutInputTexture;
    GLuint          clutTexture;
    GLuint          clutOutputTexture;
    
    // Display shader uniforms/samplers
    GLuint          displayDepthBuffer;
    GLuint          reflectionTexture;
    GLint           s_displayTexture;
    GLint           s_texture;
    GLint           s_reflectionTexture;
    GLint           s_clutTexture;
    GLint           u_borderSize;
    GLint           u_contrast;
    GLint           u_saturation;
    GLint           u_brightness;
    GLint           u_scanlineSize;
    GLint           u_scanlines;
    GLint           u_screenCurve;
    GLint           u_pixelFilterValue;
    GLint           u_rgbOffset;
    GLint           u_showVignette;
    GLint           u_vignetteX;
    GLint           u_vignetteY;
    GLint           u_showReflection;
    GLint           u_time;
    GLint           u_screenSize;
    
    Defaults        *defaults;
    
    AVCaptureSession                *captureSession;
    AVCaptureDevice                 *videoCaptureDevice;
    AVCaptureDeviceInput            *captureDeviceInput;
    AVCaptureVideoDataOutput        *captureDeviceOutput;
    dispatch_queue_t                captureQueue;
    
    CVDisplayLinkRef displayLink;
    bool             frameReady;
    bool             resizing;

}

@end

#pragma mark -

@implementation OpenGLView

- (void) awakeFromNib
{
    self.wantsLayer = YES;
    
    [self registerForDraggedTypes:@[NSURLPboardType]];
    
    defaults = [Defaults defaults];

    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        0
    };
    
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    if (!pf)
    {
        NSLog(@"No OpenGL pixel format");
    }
    
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
    CGLEnable([context CGLContextObj], kCGLCECrashOnRemovedFunctions);
    
    [self setPixelFormat:pf];
    [self setOpenGLContext:context];
    
    viewWidth = screenWidth;
    viewHeight = screenHeight;
    
    contextObj = [[self openGLContext] CGLContextObj];
    context = [self openGLContext];
    
    [self loadShaders];
    [self setupTextures];
    [self setupQuad];
    
    captureSession = [AVCaptureSession new];
    captureSession.sessionPreset = AVCaptureSessionPresetLow;
    videoCaptureDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    if (videoCaptureDevice)
    {
        NSError *error;
        captureDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:videoCaptureDevice error:&error];
        
        if (!error)
        {
            [captureSession addInput:captureDeviceInput];
            
            captureDeviceOutput = [AVCaptureVideoDataOutput new];
            [captureSession addOutput:captureDeviceOutput];
            captureDeviceOutput.videoSettings = @{ (NSString *)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA) };
            captureQueue = dispatch_queue_create("CaptureQueue", NULL);
            [captureDeviceOutput setSampleBufferDelegate:self queue:captureQueue];
            
            [defaults addObserver:self forKeyPath:@"displayShowReflection" options:NSKeyValueObservingOptionNew context:NULL];
            
            if ([[defaults valueForKey:@"displayShowReflection"] boolValue])
            {
                [captureSession startRunning];
            }
        }
        else
        {
            NSLog(@"Error getting capture device: %@", error.localizedDescription);
            [self setupDefaultReflectionTexture];
        }
    }
    else
    {
        NSLog(@"No camera device found!");
        [self setupDefaultReflectionTexture];
    }
    
    frameReady = false;
    resizing = false;
    
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (__bridge void*)self);
    
    // Set the display link for the current renderer
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    
    // Activate the display link
    CVDisplayLinkStart(displayLink);

}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

#pragma mark - Display Link

- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
    // This is called at 60Hz, so only flush changes to the screen if there is a new freame ready. This
    // If you flush the buffer contents at a different hz to the screen refresh, the window resize can
    // be jumpy
    @autoreleasepool {
        if (frameReady && !resizing)
        {
            CGLFlushDrawable(contextObj);
            frameReady = false;
        }
    }
    return kCVReturnSuccess;
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                      const CVTimeStamp* now,
                                      const CVTimeStamp* outputTime,
                                      CVOptionFlags flagsIn,
                                      CVOptionFlags* flagsOut,
                                      void* displayLinkContext)
{
    CVReturn result = [(__bridge OpenGLView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

#pragma mark - Defaults

- (void)reloadDefaults
{
    defaults = [Defaults defaults];
}

#pragma mark - Default Reflection

- (void)setupDefaultReflectionTexture
{
    NSString *path = [[NSBundle mainBundle] pathForResource:@"reflection" ofType:@"png"];
    NSData *data = [NSData dataWithContentsOfFile:path];
    NSBitmapImageRep *rep = [NSBitmapImageRep imageRepWithData:data];
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rep.size.width, rep.size.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, rep.bitmapData);
}

#pragma mark - Observers

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"displayShowReflection"])
    {
        if ([change[NSKeyValueChangeNewKey] boolValue])
        {
            [captureSession startRunning];
        }
        else
        {
            [captureSession stopRunning];
        }
    }
}

#pragma mark - Video Capture Callback

- (void)captureOutput:(AVCaptureOutput *)output didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection
{
    [[self openGLContext] makeCurrentContext];
    CGLLockContext(contextObj);

    CVImageBufferRef cameraFrame = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(cameraFrame, 0);

    int64_t bufferHeight = CVPixelBufferGetHeight(cameraFrame);
    int64_t bufferWidth = CVPixelBufferGetWidth(cameraFrame);

    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(bufferWidth), static_cast<int>(bufferHeight), 0, GL_BGRA, GL_UNSIGNED_BYTE, CVPixelBufferGetBaseAddress(cameraFrame));

    CVPixelBufferUnlockBaseAddress(cameraFrame,0);

    CGLUnlockContext(contextObj);
}

- (void) drawRect: (NSRect) theRect
{
//    [[self openGLContext] makeCurrentContext];
//    CGLLockContext(contextObj);
//    if (frameReady)
//    {
//        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//        CGLFlushDrawable(contextObj);
//    }
//    CGLUnlockContext(contextObj);
}

- (void) prepareOpenGL
{
    [self setWantsBestResolutionOpenGLSurface:YES];
    [super prepareOpenGL];

    [[self openGLContext] makeCurrentContext];
    
    NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];
}

#pragma mark - Window resizing

- (void) reshape
{
    [super reshape];

    NSRect viewRectPoints = [self bounds];
    NSRect viewRectPixels = [self convertRectToBacking:viewRectPoints];

    CGLLockContext(contextObj);
    glViewport(0, 0, viewRectPixels.size.width, viewRectPixels.size.height);
    CGLUnlockContext(contextObj);

    viewWidth = viewRectPixels.size.width;
    viewHeight = viewRectPixels.size.height;
}

- (void)renewGState
{
    // Called whenever graphics state updated (such as window resize)
    
    // OpenGL rendering is not synchronous with other rendering on the OSX.
    // Therefore, call disableScreenUpdatesUntilFlush so the window server
    // doesn't render non-OpenGL content in the window asynchronously from
    // OpenGL content, which could cause flickering.  (non-OpenGL content
    // includes the title bar and drawing done by the app with other APIs)
    [[self window] disableScreenUpdatesUntilFlush];
    
    [super renewGState];
}

- (void)viewWillStartLiveResize
{
    resizing = true;
}

- (void)viewDidEndLiveResize
{
    resizing = false;
}

#pragma mark - Shaders

- (void)loadShaders
{
    // CLUT Shader
    NSString *vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"CLUTVert" ofType:@"vsh"];
    NSString *fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"CLUTFrag" ofType:@"fsh"];
    clutShader = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    
    s_displayTexture = glGetUniformLocation(clutShader, cS_DISPLAY_TEXTURE);
    s_clutTexture = glGetUniformLocation(clutShader, cS_CLUT_TEXTURE);
    
    // Display Shader
    vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"DisplayVert" ofType:@"vsh"];
    fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"DisplayFrag" ofType:@"fsh"];
    displayShader = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    
    s_texture = glGetUniformLocation(displayShader, cS_DISPLAY_TEXTURE);
    s_reflectionTexture = glGetUniformLocation(displayShader, cS_REFLECTION_TEXTURE);
    u_borderSize = glGetUniformLocation(displayShader, cU_BORDER_SIZE);
    u_contrast = glGetUniformLocation(displayShader, cU_CONTRAST);
    u_saturation = glGetUniformLocation(displayShader, cU_SATURATION);
    u_brightness = glGetUniformLocation(displayShader, cU_BRIGHTNESS);
    u_scanlineSize = glGetUniformLocation(displayShader, cU_SCAN_LINE_SIZE);
    u_scanlines = glGetUniformLocation(displayShader, cU_SCAN_LINES);
    u_screenCurve = glGetUniformLocation(displayShader, cU_SCREEN_CURVE);
    u_pixelFilterValue = glGetUniformLocation(displayShader, cU_PIXEL_FILTER_VALUE);
    u_rgbOffset = glGetUniformLocation(displayShader, cU_RGB_OFFSET);
    u_showVignette = glGetUniformLocation(displayShader, cU_SHOW_VIGNETTE);
    u_vignetteX = glGetUniformLocation(displayShader, cU_VIGNETTE_X);
    u_vignetteY = glGetUniformLocation(displayShader, cU_VIGNETTE_Y);
    u_showReflection = glGetUniformLocation(displayShader, cU_SHOW_REFLECTION);
    u_time = glGetUniformLocation(displayShader, cU_TIME);
    u_screenSize = glGetUniformLocation(displayShader, cU_SCREEN_SIZE);
}

#pragma mark - Texture Setup

- (void)setupTextures
{
    // Setup the OpenGL texture data storage for the emulator output data. This is stored as a 1 byte per pixel array
    glGenTextures(1, &clutInputTexture);
    glBindTexture(GL_TEXTURE_2D, clutInputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Setup the OpenGL texture data storage for the CLUT data. This provides the colour to be used based on the value stored
    // in the emulator image output data and is used within the fragment shader to pick the right colour to display
    glGenTextures(1, &clutTexture);
    glBindTexture(GL_TEXTURE_2D, clutTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0, GL_RGBA, GL_FLOAT, CLUT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // Setup frame buffer to render into. The texture will be rendered into the frame buffer using NEAREST filtering and this
    // texture will be rendered to the screen using custom filtering inside the fragment shader
    glGenFramebuffers(1, &clutFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, clutFrameBuffer);
    glGenTextures(1, &clutOutputTexture);
    glBindTexture(GL_TEXTURE_2D, clutOutputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, clutOutputTexture, 0);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    
    // Bind texture 0 to the sampler in the fragment shader. This is the texture output by the emulator
    glUseProgram(displayShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, clutInputTexture);
    glUniform1i(s_displayTexture, textureUnit0);
    
    // Bind texture unit 1 to the sampler1D in the fragment shader. This is the CLUT texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, clutTexture);
    glUniform1i(s_clutTexture, textureUnit1);
    
    // Bind texture unit 2 to the texture rendered by the CLUT fragment shader, to be used when rendering output to the screen
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, clutOutputTexture);
    glUniform1i(s_texture, textureUnit2);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        NSAlert *alert = [NSAlert new];
        alert.messageText = @"Oh bother!";
        alert.informativeText = @"It was not possible to create an OpenGL off-screen frame buffer.";
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        exit(1);
    }
    
    glGenTextures(1, &reflectionTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glUniform1i(s_reflectionTexture, 3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

- (void)setupQuad
{
    // Generate the Vertex array and bind the vertex buffer to it.
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)12);
    glEnableVertexAttribArray(0);
}

#pragma mark - Texture update

- (void)updateTextureData:(void *)displayBuffer
{
    [[self openGLContext] makeCurrentContext];
    CGLLockContext(contextObj);
 
    // Render the output to a texture which has the default dimentions of the output image
    glBindFramebuffer(GL_FRAMEBUFFER, clutFrameBuffer);
    glViewport(0, 0, screenWidth, screenHeight);
    glUseProgram(clutShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, clutInputTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_RED, GL_UNSIGNED_BYTE, displayBuffer);
    glUniform1i(s_displayTexture, textureUnit0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, clutTexture);
    glUniform1i(s_clutTexture, textureUnit1);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Render the texture to the actual screen, this time using the size of the screen as the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewWidth, viewHeight);
    glUseProgram(displayShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, clutOutputTexture);
    glUniform1i(s_texture, textureUnit0);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glUniform1i(s_reflectionTexture, 3);

    // Update uniforms in the shader
    glProgramUniform1i(displayShader, u_borderSize, defaults.displayBorderSize);
    glProgramUniform1f(displayShader, u_contrast, defaults.displayContrast);
    glProgramUniform1f(displayShader, u_saturation, defaults.displaySaturation);
    glProgramUniform1f(displayShader, u_brightness, defaults.displayBrightness);
    glProgramUniform1f(displayShader, u_scanlineSize, defaults.displayScanLineSize);
    glProgramUniform1f(displayShader, u_scanlines, defaults.displayScanLines);
    glProgramUniform1f(displayShader, u_screenCurve, defaults.displayCurvature);
    glProgramUniform1f(displayShader, u_pixelFilterValue, defaults.displayPixelFilterValue);
    glProgramUniform1f(displayShader, u_rgbOffset, defaults.displayRGBOffset);
    glProgramUniform1i(displayShader, u_showVignette, defaults.displayShowVignette);
    glProgramUniform1f(displayShader, u_vignetteX, defaults.displayVignetteX);
    glProgramUniform1f(displayShader, u_vignetteY, defaults.displayVignetteY);
    glProgramUniform1i(displayShader, u_showReflection, defaults.displayShowReflection);
    glProgramUniform1f(displayShader, u_time, CACurrentMediaTime());
    glProgramUniform2f(displayShader, u_screenSize, float(self.frame.size.width), float(self.frame.size.height));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    CGLUnlockContext(contextObj);
    frameReady = true;
}

#pragma mark - Load Shaders

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else
    {
        printf("Cannot open %s.\n", vertex_file_path);
        getchar();
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
    {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }
    
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
    {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    
    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
    {
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}

#pragma mark - View Drag/Drop

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    NSPasteboard *pBoard;
    NSDragOperation sourceDragMask;
    sourceDragMask = [sender draggingSourceOperationMask];
    pBoard = [sender draggingPasteboard];
    
    if ([[pBoard types] containsObject:NSFilenamesPboardType])
    {
        if (sourceDragMask * NSDragOperationCopy)
        {
            NSURL *fileURL = [NSURL URLFromPasteboard:pBoard];
            if ([[fileURL.pathExtension uppercaseString] isEqualToString:cZ80_EXTENSION] ||
                [[fileURL.pathExtension uppercaseString] isEqualToString:cSNA_EXTENSION] ||
                [[fileURL.pathExtension uppercaseString] isEqualToString:cTAP_EXTENSION])
            {
                return NSDragOperationCopy;
            }
            else
            {
                return NSDragOperationNone;
            }
        }
    }
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSPasteboard *pBoard = [sender draggingPasteboard];
    if ([[pBoard types] containsObject:NSURLPboardType])
    {
        NSURL *fileURL = [NSURL URLFromPasteboard:pBoard];
        if ([[fileURL.pathExtension uppercaseString] isEqualToString:cZ80_EXTENSION] ||
            [[fileURL.pathExtension uppercaseString] isEqualToString:cSNA_EXTENSION] ||
            [[fileURL.pathExtension uppercaseString] isEqualToString:cTAP_EXTENSION])
        {
            EmulationViewController *emulationViewController = (EmulationViewController *)[self.window contentViewController];
            [emulationViewController loadFileWithURL:fileURL addToRecent:YES];
            return YES;
        }
    }
    return NO;
}

- (void)mouseDown:(NSEvent *)event
{
    [self.window performWindowDragWithEvent:event];
}

@end
