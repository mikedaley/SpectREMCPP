//
//  GameScene.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationScene.h"

@implementation EmulationScene {
    SKSpriteNode *_emulationScreen;
}

- (void)didMoveToView:(SKView *)view {
    // Get label node from scene and store it for use later
    _emulationScreen = (SKSpriteNode *)[self childNodeWithName:@"//emulationScreen"];
    _emulationScreenTexture = [SKMutableTexture mutableTextureWithSize:CGSizeMake(320, 256)];
    _emulationScreen.texture = _emulationScreenTexture;
}

- (void)touchDownAtPoint:(CGPoint)pos {
}

- (void)touchMovedToPoint:(CGPoint)pos {
}

- (void)touchUpAtPoint:(CGPoint)pos {
}

- (void)keyDown:(NSEvent *)theEvent {
    switch (theEvent.keyCode) {
        case 0x31 /* SPACE */:
            // Run 'Pulse' action from 'Actions.sks'
            break;
            
        default:
            NSLog(@"keyDown:'%@' keyCode: 0x%02X", theEvent.characters, theEvent.keyCode);
            break;
    }
}

- (void)mouseDown:(NSEvent *)theEvent {

}
- (void)mouseDragged:(NSEvent *)theEvent {

}
- (void)mouseUp:(NSEvent *)theEvent {

}

-(void)update:(CFTimeInterval)currentTime {
    // Called before each frame is rendered
}

@end
