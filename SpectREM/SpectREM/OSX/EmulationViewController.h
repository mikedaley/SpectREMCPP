//
//  ViewController.h
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <MetalKit/MetalKit.h>
#import "Defaults.h"

@class EmulationScene;
@class AudioCore;

#pragma mark - Constants

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
@property (strong)              AudioCore           *audioCore;
@property (weak)    IBOutlet    NSVisualEffectView  *infoEffectsView;

#pragma mark - Methods

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent;

- (void)stopAudioCore;
- (void)startAudioCore;
- (void)updateDisplay;

@end
