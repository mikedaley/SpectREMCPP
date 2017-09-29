//
//  OpenGLView.h
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <GLKit/GLKit.h>

@class OpenGLRenderer;

@interface OpenGLView : NSOpenGLView

@property (strong, nonatomic) OpenGLRenderer *renderer;

- (void)render;

@end
