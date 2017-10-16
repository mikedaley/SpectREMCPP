//
//  OpenGLView.m
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "OpenGLView.h"
#import "EmulationViewController.h"
#import "Defaults.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#pragma mark - Constants

const GLfloat quad[] = {
    //X      Y      Z        U      V
    -1.0f,   1.0f,  0.0f,    0.0f,  1.0f,
    -1.0f,  -1.0f,  0.0f,    0.0f,  0.0f,
     1.0f,  -1.0f,  0.0f,    1.0f,  0.0f,
     1.0f,  -1.0f,  0.0f,    1.0f,  0.0f,
    -1.0f,   1.0f,  0.0f,    0.0f,  1.0f,
     1.0f,   1.0f,  0.0f,    1.0f,  1.0f
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
    { 0.0, 0.0, 0.0, 1.0 },
    { 0.0, 0.0, normalColor, 1.0 },
    { normalColor, 0.0, 0.0, 1.0 },
    { normalColor, 0.0, normalColor, 1.0 },
    { 0.0, normalColor, 0.0, 1.0 },
    { 0.0, normalColor, normalColor, 1.0 },
    { normalColor, normalColor, 0.0, 1.0 },
    { normalColor, normalColor, normalColor, 1.0 },
    
    // Bright colours
    { 0.0, 0.0, 0.0, 1.0 },
    { 0.0, 0.0, brightColor, 1.0 },
    { brightColor, 0.0, 0.0, 1.0 },
    { brightColor, 0.0, brightColor, 1.0 },
    { 0.0, brightColor, 0.0, 1.0 },
    { 0.0, brightColor, brightColor, 1.0 },
    { brightColor, brightColor, 0.0, 1.0 },
    { brightColor, brightColor, brightColor, 1.0 }
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
    
    GLuint          viewWidth;
    GLuint          viewHeight;
    
    GLuint          vertexBuffer;
    GLuint          vertexArray;
    GLuint          displayShader;
    GLuint          clutShader;
    GLuint          textureName;
    GLuint          clutTextureName;
    
    GLuint          s_displayTexture;
    GLuint          s_texture;
    GLuint          s_clutTexture;
    GLuint          u_borderSize;
    GLuint          u_contrast;
    GLuint          u_saturation;
    GLuint          u_brightness;
    GLuint          u_scanlineSize;
    GLuint          u_scanlines;
    GLuint          u_screenCurve;
    GLuint          u_pixelFilterValue;
    GLuint          u_rgbOffset;
    
    GLuint          u_time;
    
    GLuint          frameBufferName;
    GLuint          renderedTexture;
    
    Defaults        *defaults;
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
        NSOpenGLPFADepthSize, 16,
        NSOpenGLPFAOpenGLProfile,
        NSOpenGLProfileVersion4_1Core,
        0
    };
    
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    if (!pf)
    {
        NSLog(@"No OpenGL pixel format");
    }
    
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
    
    // When we're using a CoreProfile context, crash if we call a legacy OpenGL function
    // This will make it much more obvious where and when such a function call is made so
    // that we can remove such calls.
    // Without this we'd simply get GL_INVALID_OPERATION error for calling legacy functions
    // but it would be more difficult to see where that function was called.
    CGLEnable([context CGLContextObj], kCGLCECrashOnRemovedFunctions);
    
    [self setPixelFormat:pf];
    [self setOpenGLContext:context];
    
    NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    
    viewWidth = screenWidth;
    viewHeight = screenHeight;
    
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    [self loadShaders];
    [self setupTexture];
    [self setupQuad];
}

- (void) drawRect: (NSRect) theRect
{
    [[self openGLContext] makeCurrentContext];
    CGLContextObj ctxObj = [[self openGLContext] CGLContextObj];
    CGLLockContext(ctxObj);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    CGLFlushDrawable(ctxObj);
    CGLUnlockContext(ctxObj);
}

- (void) prepareOpenGL
{
    [self setWantsBestResolutionOpenGLSurface:YES];
    [super prepareOpenGL];
    [[self openGLContext] makeCurrentContext];
    
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

- (void)setupQuad
{
    // Generate the Vertex array and bind the vertex buffer to it.
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    // Make sure we are using the shader program we have compiled
    glUseProgram(displayShader);
    
    // Bind texture 0 to the sampler2D in the fragment shader. This is the texture output by the emulator
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glUniform1i(s_displayTexture, textureUnit0);

    // Bind texture unit 1 to the sampler1D in the fragment shader. This is the CLUT texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, clutTextureName);
    glUniform1i(s_clutTexture, textureUnit1);

    // Bind texture unit 2 to the texture rendered by the CLUT fragment shader
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glUniform1i(s_texture, textureUnit2);

    // Enable and configure the vertex and texture pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)12);
    glEnableVertexAttribArray(0);
}

- (void)loadShaders
{
    NSString *vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"Display" ofType:@"vsh"];
    NSString *fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"CLUT" ofType:@"fsh"];
    clutShader = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);

    s_displayTexture = glGetUniformLocation(clutShader, "displayTexture");
    s_clutTexture = glGetUniformLocation(clutShader, "clutTexture");

    vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"Display" ofType:@"vsh"];
    fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"Display" ofType:@"fsh"];
    displayShader = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    
    s_texture = glGetUniformLocation(displayShader, "displayTexture");
    u_borderSize = glGetUniformLocation(displayShader, "u_borderSize");
    u_contrast = glGetUniformLocation(displayShader, "u_contrast");
    u_saturation = glGetUniformLocation(displayShader, "u_saturation");
    u_brightness = glGetUniformLocation(displayShader, "u_brightness");
    u_scanlineSize = glGetUniformLocation(displayShader, "u_scanlineSize");
    u_scanlines = glGetUniformLocation(displayShader, "u_scanlines");
    u_screenCurve = glGetUniformLocation(displayShader, "u_screenCurve");
    u_pixelFilterValue = glGetUniformLocation(displayShader, "u_pixelFilterValue");
    u_rgbOffset = glGetUniformLocation(displayShader, "u_rgbOffset");
    u_time = glGetUniformLocation(displayShader, "u_time");
}

- (void)setupTexture
{
    // Setup the OpenGL texture data storage for the emulator output data. This is stored as a 1 byte per pixel array
    glGenTextures(1, &textureName);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Setup the OpenGL texture data storage for the CLUT data. This provides the colour to be used based on the value stored
    // in the emulator image output data and is used within the fragment shader to pick the right colour to display
    glGenTextures(1, &clutTextureName);
    glBindTexture(GL_TEXTURE_2D, clutTextureName);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0, GL_RGBA, GL_FLOAT, CLUT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // Setup frame buffer to render into. The texture will be rendered into the frame buffer using NEAREST filtering and then this
    // texture will be rendered to the screen using custom filtering inside the fragment shader
    glGenFramebuffers(1, &frameBufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"NO FRAME BUFFER");
    }
}

- (void)updateTextureData:(void *)displayBuffer
{
    [[self openGLContext] makeCurrentContext];
    CGLLockContext([[self openGLContext] CGLContextObj]);
 
    // Render the output to a texture which has the default dimentions of the output image
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
    glViewport(0, 0, screenWidth, screenHeight);
    glUseProgram(clutShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glUniform1i(s_displayTexture, 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_RED, GL_UNSIGNED_BYTE, displayBuffer);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, clutTextureName);
    glUniform1i(s_clutTexture, 1);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Render the texture to the actual screen, this time using the size of the screen as the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewWidth, viewHeight);
    glUseProgram(displayShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glUniform1i(s_texture, 0);

    // Apply uniforms to the shader
    glProgramUniform1f(displayShader, u_borderSize, defaults.displayBorderSize);
    glProgramUniform1f(displayShader, u_contrast, defaults.displayContrast);
    glProgramUniform1f(displayShader, u_saturation, defaults.displaySaturation);
    glProgramUniform1f(displayShader, u_brightness, defaults.displayBrightness);
    glProgramUniform1f(displayShader, u_scanlineSize, defaults.displayScanLineSize);
    glProgramUniform1f(displayShader, u_scanlines, defaults.displayScanLines);
    glProgramUniform1f(displayShader, u_screenCurve, defaults.displayCurvature);
    glProgramUniform1f(displayShader, u_pixelFilterValue, defaults.displayPixelFilterValue);
    glProgramUniform1f(displayShader, u_rgbOffset, defaults.displayRGBOffset);
    glProgramUniform1f(displayShader, u_time, CACurrentMediaTime());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    CGLFlushDrawable([[self openGLContext] CGLContextObj]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
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
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
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
