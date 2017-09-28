//
//  OpenGLRenderer.m
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "OpenGLRenderer.h"

// Vertices of the triangle
GLfloat posArray[] = {
    
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
    GLuint          _vaoName;
    GLuint          _vboName;
    GLKBaseEffect*  _baseEffect;
    
    GLuint          _viewWidth;
    GLuint          _viewHeight;
}
@end

@implementation OpenGLRenderer

#pragma mark - Overridden Methods

- (id) init
{
    if((self = [super init]))
    {
        NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
        
        _viewWidth = 100;
        _viewHeight = 100;
        
        GLKTextureInfo *texture;
        NSImage *image = [NSImage imageNamed:@"reflection"];
        texture = [GLKTextureLoader textureWithContentsOfData:[image TIFFRepresentation] options:nil error:nil];
        
        // Create a vertex array object (VAO)
        glGenVertexArrays(1, &_vaoName);
        glBindVertexArray(_vaoName);
        
        // create a BaseEffect (it will supply the vertex & fragment shaders)
        _baseEffect = [[GLKBaseEffect alloc] init];
//        _baseEffect.useConstantColor = GL_TRUE;
//        _baseEffect.constantColor = GLKVector4Make(0.48, 0.34, 0.01, 1.0);
        _baseEffect.texture2d0.name = texture.name;
        _baseEffect.texture2d0.target = texture.target;
        
        // Create, Bind, Allocate and Load data into a vertex buffer object (VBO)
        glGenBuffers(1, &_vboName);
        glBindBuffer(GL_ARRAY_BUFFER, _vboName);
        glBufferData(GL_ARRAY_BUFFER, sizeof(posArray), posArray, GL_STATIC_DRAW);
        
        // set the clear color
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        
        // Enable simple blending of the image with the background
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable( GL_BLEND );
    }
    return self;
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
    // prepare the BaseEffect
    [_baseEffect prepareToDraw];
    
    // clear the buffer
    glClear(GL_COLOR_BUFFER_BIT);
    
    // enable the vertex data
    glEnableVertexAttribArray(GLKVertexAttribPosition);
    glVertexAttribPointer(GLKVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    
    glEnableVertexAttribArray(GLKVertexAttribTexCoord0);
    glVertexAttribPointer(GLKVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char *)12);
    
    // Draw the triangle
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}
@end
