//
//  ViewController.h
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <SpriteKit/SpriteKit.h>
#import <GameplayKit/GameplayKit.h>

@interface EmulationViewController : NSViewController

#pragma mark - Properties

@property (assign) IBOutlet SKView *skView;

#pragma mark - Methods

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent;

int* resizePixels(int* pixels,int w1,int h1,int w2,int h2);

@end

