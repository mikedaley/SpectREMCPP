//
//  MetalView.h
//  SpectREM
//
//  Created by Mike on 20/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#import <MetalKit/MetalKit.h>

@interface MetalRenderer : NSObject <MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView;

// Called once per emulation frame to update the texture with the emulators screen data
- (void)updateTextureData:(void *_Nonnull)displayBuffer clutData:(void *_Nonnull)clutBuffer;

@end
