//
//  OpenGLRenderer.h
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>

enum {
    POS_ATTRIB_IDX = 0,
    TEXCOORD_ATTRIB_IDX = 1
};

@interface OpenGLRenderer : NSObject

#pragma mark - Public Methods

- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height;
- (void) render;
- (void)updateTextureData:(void *)displayBuffer;

@end
