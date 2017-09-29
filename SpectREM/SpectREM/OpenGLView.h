//
//  OpenGLView.h
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OpenGLRenderer.h"
#import "OpenGLView.h"

@interface OpenGLView : NSOpenGLView <NSDraggingDestination>

@property (strong, nonatomic) OpenGLRenderer *renderer;

- (void)render;
- (void)updateTextureData:(void *)displayBuffer;

@end
