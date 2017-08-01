//
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationViewController.h"
#import "EmulationScene.h"

@implementation EmulationViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Load the SKScene from 'GameScene.sks'
    EmulationScene *scene = (EmulationScene *)[SKScene nodeWithFileNamed:@"Scene"];
    
    // Set the scale mode to scale to fit the window
    scene.scaleMode = SKSceneScaleModeAspectFill;
    
    // Present the scene
    [self.skView presentScene:scene];
    
    self.skView.showsFPS = YES;
    self.skView.showsNodeCount = YES;
}

@end
