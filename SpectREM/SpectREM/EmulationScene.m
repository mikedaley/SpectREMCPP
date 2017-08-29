//
//  GameScene.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationScene.h"

@implementation EmulationScene

- (void)didMoveToView:(SKView *)view
{
    _emulationScreenTexture = [SKMutableTexture mutableTextureWithSize:CGSizeMake(320, 256)];
    _emulationScreenTexture.filteringMode = SKTextureFilteringLinear;
    
    _backingTexture = [SKMutableTexture mutableTextureWithSize:CGSizeMake(320, 256)];
    
    _emulationScreen = (SKSpriteNode *)[self childNodeWithName:@"//emulationScreen"];
    _emulationScreen.texture = _emulationScreenTexture;
    
    _backingNode = (SKSpriteNode *)[self childNodeWithName:@"//backingNode"];
}

-(void)update:(CFTimeInterval)currentTime
{
    // Called before each frame is rendered
}

@end
