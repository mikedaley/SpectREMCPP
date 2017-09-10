//
//  ViewController.h
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <SpriteKit/SpriteKit.h>

@class EmulationScene;

enum
{
    cZ80_SNAPSHOT_TYPE = 0,
    cSNA_SNAPSHOT_TYPE
};

@interface EmulationViewController : NSViewController

#pragma mark - Properties

@property (assign)  IBOutlet    SKView              *skView;
@property (strong)              EmulationScene      *scene;
@property (weak)    IBOutlet    NSVisualEffectView  *configEffectsView;
@property (weak)    IBOutlet    NSScrollView        *configScrollView;

#pragma mark - Methods

- (void *)getDisplayBuffer;
- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent;
- (void)audioCallback:(int)inNumberFrames buffer:(short *)buffer;

// Methods used to get informaiton from the current machine for the tape browser
- (NSInteger)numberOfTapeBlocks;
- (NSString *)blockNameForTapeBlockIndex:(NSInteger)blockIndex;
- (NSInteger)selectedTapeBlock;
- (BOOL)isTapePlaying;
- (void)setCurrentTapeBlock:(NSInteger)blockIndex;

@end
