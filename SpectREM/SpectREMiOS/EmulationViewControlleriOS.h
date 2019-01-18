//
//  GameViewController.h
//  SpectREMiOS
//
//  Created by Michael Daley on 07/01/2019.
//  Copyright © 2019 71Squared Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "MetalRenderer.h"

@class AudioCore;

// Our iOS view controller
@interface EmulationViewControlleriOS : UIViewController

@property (weak, nonatomic) IBOutlet UIVisualEffectView *configView;

@property (strong)              AudioCore           *audioCore;

- (void)audioCallback:(int)inNumberFrames buffer:(int16_t *)buffer;
- (void)pauseMachine;
- (void)startMachine;

@end
