//
//  OpenGLView.h
//  SpectREM
//
//  Created by Mike Daley on 28/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import "OpenGLRenderer.h"
#import "OpenGLView.h"

@interface OpenGLView : NSOpenGLView <NSDraggingDestination, AVCaptureVideoDataOutputSampleBufferDelegate>

@property (strong, nonatomic) OpenGLRenderer *renderer;

// Called once per emulation frame to update the texture with the emulators screen data
- (void)updateTextureData:(void *)displayBuffer;

@end
