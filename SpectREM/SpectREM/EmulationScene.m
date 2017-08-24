//
//  GameScene.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationScene.h"

@implementation EmulationScene
{
    SKSpriteNode *_emulationScreen;
}

- (void)didMoveToView:(SKView *)view
{
    _emulationScreenTexture = [SKMutableTexture mutableTextureWithSize:CGSizeMake(320, 256)];
    _emulationScreenTexture.filteringMode = SKTextureFilteringLinear;
    
    _emulationScreen = (SKSpriteNode *)[self childNodeWithName:@"//displayNode"];
    _emulationScreen.texture = _emulationScreenTexture;
}

-(void)update:(CFTimeInterval)currentTime
{
    // Called before each frame is rendered
}

@end
