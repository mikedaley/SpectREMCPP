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
     3-----2 <-----  1
     |\    |
     |  \  | <-----  0
     |    \|
     0-----1 <----- -1

     ^  ^  ^
     |  |  |
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
const GLfloat normalColor = 208.0 / 255.0;
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

const GLuint screenWidth = 320;
const GLuint screenHeight = 256;

#pragma mark - Private Ivars

@interface OpenGLView ()
{
    NSTrackingArea *trackingArea;
    NSWindowController *windowController;
    CGLContextObj contextObj;
    
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
    
    Defaults        *defaults;
    
    AVCaptureSession                *captureSession;
    AVCaptureDevice                 *videoCaptureDevice;
    AVCaptureDeviceInput            *captureDeviceInput;
    AVCaptureVideoDataOutput        *captureDeviceOutput;
    dispatch_queue_t                captureQueue;

}

@end

#pragma mark -

@implementation OpenGLView

- (void) awakeFromNib
{
    [self registerForDraggedTypes:@[NSURLPboardType]];
    
    defaults = [Defaults defaults];

    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAOpenGLProfile,
        NSOpenGLProfileVersion3_2Core,
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
        }
    }
    {
        NSLog(@"No camera device found!");
    }
}

- (void)reloadDefaults
{
    defaults = [Defaults defaults];
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
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<int>(bufferWidth), static_cast<int>(bufferHeight), GL_BGRA, GL_UNSIGNED_BYTE, CVPixelBufferGetBaseAddress(cameraFrame));

    CVPixelBufferUnlockBaseAddress(cameraFrame,0);

    CGLUnlockContext(contextObj);
}

- (void) drawRect: (NSRect) theRect
{
    [[self openGLContext] makeCurrentContext];
    CGLLockContext(contextObj);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    CGLUnlockContext(contextObj);
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

- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height
{
    glViewport(0, 0, width, height);
    viewWidth = width;
    viewHeight = height;
}

- (void) reshape
{
    [super reshape];
    
    CGLLockContext([[self openGLContext] CGLContextObj]);
    
    NSRect viewRectPoints = [self bounds];
    NSRect viewRectPixels = [self convertRectToBacking:viewRectPoints];
    
    [self resizeWithWidth:viewRectPixels.size.width
                AndHeight:viewRectPixels.size.height];
    
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (void)renewGState
{
    [[self window] disableScreenUpdatesUntilFlush];
    [super renewGState];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

#pragma mark - Renderer

- (void)loadShaders
{
    // CLUT Shader
    NSString *vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"CLUTVert" ofType:@"vsh"];
    NSString *fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"CLUTFrag" ofType:@"fsh"];
    clutShader = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    
    s_displayTexture = glGetUniformLocation(clutShader, "s_displayTexture");
    s_clutTexture = glGetUniformLocation(clutShader, "s_clutTexture");
    
    // Display Shader
    vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"DisplayVert" ofType:@"vsh"];
    fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"DisplayFrag" ofType:@"fsh"];
    displayShader = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    
    s_texture = glGetUniformLocation(displayShader, "s_displayTexture");
    s_reflectionTexture = glGetUniformLocation(displayShader, "s_reflectionTexture");
    u_borderSize = glGetUniformLocation(displayShader, "u_borderSize");
    u_contrast = glGetUniformLocation(displayShader, "u_contrast");
    u_saturation = glGetUniformLocation(displayShader, "u_saturation");
    u_brightness = glGetUniformLocation(displayShader, "u_brightness");
    u_scanlineSize = glGetUniformLocation(displayShader, "u_scanlineSize");
    u_scanlines = glGetUniformLocation(displayShader, "u_scanlines");
    u_screenCurve = glGetUniformLocation(displayShader, "u_screenCurve");
    u_pixelFilterValue = glGetUniformLocation(displayShader, "u_pixelFilterValue");
    u_rgbOffset = glGetUniformLocation(displayShader, "u_rgbOffset");
    u_showVignette = glGetUniformLocation(displayShader, "u_showVignette");
    u_vignetteX = glGetUniformLocation(displayShader, "u_vignetteX");
    u_vignetteY = glGetUniformLocation(displayShader, "u_vignetteY");
    u_showReflection = glGetUniformLocation(displayShader, "u_showReflection");
    u_time = glGetUniformLocation(displayShader, "u_time");
}

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
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

- (void)updateTextureData:(void *)displayBuffer
{
    [[self openGLContext] makeCurrentContext];
    CGLLockContext(contextObj);
 
    glClear(GL_COLOR_BUFFER_BIT);
    
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
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    CGLFlushDrawable(contextObj);
    CGLUnlockContext(contextObj);
}

#pragma mark - Load Shaders

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        printf("Cannot open %s.\n", vertex_file_path);
        getchar();
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
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
    if ( InfoLogLength > 0 ){
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
    if ( InfoLogLength > 0 ){
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
    if ( InfoLogLength > 0 ){
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

#pragma mark - Drag/Drop

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
            if ([[fileURL.pathExtension uppercaseString] isEqualToString:@"Z80"] ||
                [[fileURL.pathExtension uppercaseString] isEqualToString:@"SNA"] ||
                [[fileURL.pathExtension uppercaseString] isEqualToString:@"TAP"])
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
        if ([[fileURL.pathExtension uppercaseString] isEqualToString:@"Z80"] ||
            [[fileURL.pathExtension uppercaseString] isEqualToString:@"SNA"] ||
            [[fileURL.pathExtension uppercaseString] isEqualToString:@"TAP"])
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
