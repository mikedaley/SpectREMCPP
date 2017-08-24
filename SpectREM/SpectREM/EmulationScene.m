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
    
    _emulationScreen = (SKSpriteNode *)[self childNodeWithName:@"//display"];
    _emulationScreen.texture = _emulationScreenTexture;
}

- (void)keyDown:(NSEvent *)theEvent
{
    switch (theEvent.keyCode)
    {
        case 0x31 /* SPACE */:
            // Run 'Pulse' action from 'Actions.sks'
            break;
            
        default:
            NSLog(@"keyDown:'%@' keyCode: 0x%02X", theEvent.characters, theEvent.keyCode);
            break;
    }
}

-(void)update:(CFTimeInterval)currentTime
{
    // Called before each frame is rendered
}

@end
