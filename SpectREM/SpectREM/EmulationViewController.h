//
//  ViewController.h
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <SpriteKit/SpriteKit.h>
#import "Defaults.h"

@class EmulationScene;

enum
{
    cZ80_SNAPSHOT_TYPE = 0,
    cSNA_SNAPSHOT_TYPE
};

@interface EmulationViewController : NSViewController

#pragma mark - Properties

@property (weak)    IBOutlet    NSVisualEffectView  *configEffectsView;
@property (weak)    IBOutlet    NSScrollView        *configScrollView;
@property (strong)              Defaults            *defaults;

#pragma mark - Methods

- (void *)getDisplayBuffer;
- (void *)getRGBADisplayBuffer;
- (BOOL)getDisplayReady;

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent;
- (void)audioCallback:(int)inNumberFrames buffer:(unsigned short *)buffer;

// Methods used to get informaiton from the current machine for the tape browser
- (NSInteger)tapeNumberOfblocks;
- (NSString *)tapeBlockTypeForIndex:(NSInteger)blockIndex;
- (NSString *)tapeFilenameForIndex:(NSInteger)blockIndex;
- (int)tapeAutostartLineForIndex:(NSInteger)blockIndex;
- (unsigned short)tapeBlockStartAddressForIndex:(NSInteger)blockIndex;
- (unsigned short)tapeBlockLengthForIndex:(NSInteger)blockIndex;
- (NSInteger)tapeCurrentBlock;
- (BOOL)tapeIsplaying;
- (void)tapeSetCurrentBlock:(NSInteger)blockIndex;

- (IBAction)showTapeBrowser:(id)sender;
- (IBAction)startPlayingTape:(id)sender;
- (IBAction)stopPlayingTape:(id)sender;
- (IBAction)rewindTape:(id)sender;
- (IBAction)ejectTape:(id)sender;
- (IBAction)saveTape:(id)sender;

@end
