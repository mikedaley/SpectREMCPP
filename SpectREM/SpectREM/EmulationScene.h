//
//  GameScene.h
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <SpriteKit/SpriteKit.h>

@class EmulationViewController;

@interface EmulationScene : SKScene

@property (strong, nonatomic) SKMutableTexture *emulationScreenTexture;
@property (strong, nonatomic) SKMutableTexture *backingTexture;

@property (strong, nonatomic) SKSpriteNode *emulationScreen;
@property (strong, nonatomic) SKSpriteNode *backingNode;

@property (strong, nonatomic) EmulationViewController *emulationViewController;

@property (assign, nonatomic) CGFloat borderSize;

@end
