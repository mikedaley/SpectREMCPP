//
//  TapeBrowserViewController.h
//  SpectREM
//
//  Created by Michael Daley on 10/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//
#ifndef TapeBrowserViewController_h
#define TapeBrowserViewController_h

#import <Cocoa/Cocoa.h>
#include "EmulationController.hpp"

@interface TapeBrowserViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource>

@property (nonatomic) EmulationController *emulationController;

- (IBAction)playTape:(id)sender;
- (IBAction)stopTape:(id)sender;
- (IBAction)rewindTape:(id)sender;

void tapeStatusCallback(int blockIndex, int bytes, int action);

#endif

@end
