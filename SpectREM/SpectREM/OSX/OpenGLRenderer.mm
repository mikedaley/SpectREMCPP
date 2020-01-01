//
//  OpenGLRenderer.m
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "OpenGLRenderer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Vertices of the triangle
GLfloat quad[] = {
    
    -1.0f,   1.0f,  0.0f,    0.0f,  1.0f,
    -1.0f,  -1.0f,  0.0f,    0.0f,  0.0f,
     1.0f,  -1.0f,  0.0f,    1.0f,  0.0f,
     1.0f,  -1.0f,  0.0f,    1.0f,  0.0f,
    -1.0f,   1.0f,  0.0f,    0.0f,  1.0f,
     1.0f,   1.0f,  0.0f,    1.0f,  1.0f
};

#pragma mark - Private Ivars

@interface OpenGLRenderer ()
{
    GLuint          _vertexBuffer;
    GLuint          _vertexArray;
    
    GLuint          _viewWidth;
    GLuint          _viewHeight;

    GLuint          _shaderProgName;
    GLuint          _textureName;
    GLuint          _textureID;

}
@end

@implementation OpenGLRenderer

#pragma mark - Overridden Methods

- (id) init
{
    if((self = [super init]))
    {
        NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
        
        _viewWidth = 320;
        _viewHeight = 256;

        // set the clear color
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
 
        glGenVertexArrays(1, &_vertexArray);
        glBindVertexArray(_vertexArray);

        [self loadShaders];
        [self loadQuad];
        [self loadTexture];
    }
    return self;
}

- (void)loadQuad
{
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
}

- (void)loadShaders
{
    NSString *vertexShaderPath = [[NSBundle mainBundle] pathForResource:@"Display" ofType:@"vsh"];
    NSString *fragmentShaderPath = [[NSBundle mainBundle] pathForResource:@"Display" ofType:@"fsh"];
    _shaderProgName = LoadShaders([vertexShaderPath UTF8String], [fragmentShaderPath UTF8String]);
    _textureID = glGetUniformLocation(_shaderProgName, "mySampler");
}

- (void)loadTexture
{
    // Create a texture object to apply to model
    glGenTextures(1, &_textureName);
    glBindTexture(GL_TEXTURE_2D, _textureName);
}

- (void)updateTextureData:(void *)displayBuffer
{
    glBindTexture(GL_TEXTURE_2D, _textureName);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, displayBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
}

#pragma mark - Public Methods

- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height
{
    // adjust the Viewport
    glViewport(0, 0, width, height);
    _viewWidth = width;
    _viewHeight = height;
}

- (void) render
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(_shaderProgName);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _textureName);
    glUniform1i(_textureID, 0);

    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)12);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

// Load shaders provided
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

@end
