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

#pragma mark - Private Ivars

@interface OpenGLView ()
{
    NSTrackingArea *trackingArea;
    NSWindowController *windowController;
    
    GLuint          viewWidth;
    GLuint          viewHeight;
    
    GLuint          vertexBuffer;
    GLuint          vertexArray;
    GLuint          shaderProgName;
    GLuint          textureName;
    
    GLuint          textureID;
    GLuint          u_borderSize;
    
    Defaults        *defaults;
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
        NSOpenGLPFADepthSize, 24,
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
    
    viewWidth = 320;
    viewHeight = 256;
    
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
    
    glClear(GL_COLOR_BUFFER_BIT);

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
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
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
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glUseProgram(shaderProgName);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glUniform1i(textureID, 0);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)12);
}

- (void)loadShaders
{
    NSString *vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"Display" ofType:@"vsh"];
    NSString *fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"Display" ofType:@"fsh"];
    shaderProgName = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    textureID = glGetUniformLocation(shaderProgName, "mySampler");
    u_borderSize = glGetUniformLocation(shaderProgName, "borderSize");
}

- (void)setupTexture
{
    glGenTextures(1, &textureName);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 320, 256, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

- (void)updateTextureData:(void *)displayBuffer
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindTexture(GL_TEXTURE_2D, textureName);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 256, GL_RED, GL_UNSIGNED_BYTE, displayBuffer);

    [[self openGLContext] makeCurrentContext];
    CGLLockContext([[self openGLContext] CGLContextObj]);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glProgramUniform1f(shaderProgName, u_borderSize, defaults.displayBorderSize);
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
